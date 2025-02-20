#include <algorithm>
#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "grammar.hpp"
#include "ll1_parser.hpp"
#include "symbol_table.hpp"
#include "tabulate.hpp"

LL1Parser::LL1Parser(Grammar gr) : gr_(std::move(gr)) {}

bool LL1Parser::CreateLL1Table() {
    ComputeFirstSets();
    size_t nrows{gr_.g_.size()};
    ll1_t_.reserve(nrows);
    bool has_conflict{false};
    for (const auto& rule : gr_.g_) {
        std::unordered_map<std::string, std::vector<production>> column;
        for (const production& p : rule.second) {
            std::unordered_set<std::string> ds =
                PredictionSymbols(rule.first, p);
            column.reserve(ds.size());
            for (const std::string& symbol : ds) {
                auto& cell = column[symbol];
                if (!cell.empty()) {
                    has_conflict = true;
                }
                cell.push_back(p);
            }
        }
        ll1_t_.insert({rule.first, column});
    }
    return !has_conflict;
}

void LL1Parser::First(std::span<const std::string>     rule,
                      std::unordered_set<std::string>& result) {
    if (rule.empty() || (rule.size() == 1 && rule[0] == gr_.st_.EPSILON_)) {
        result.insert(gr_.st_.EPSILON_);
        return;
    }

    if (gr_.st_.IsTerminal(rule[0])) {
        // EOL cannot be in first sets, if we reach EOL it means that the axiom
        // is nullable, so epsilon is included instead
        if (rule[0] == gr_.st_.EOL_) {
            result.insert(gr_.st_.EPSILON_);
            return;
        }
        result.insert(rule[0]);
        return;
    }

    const std::unordered_set<std::string>& fii = first_sets[rule[0]];
    for (const auto& s : fii) {
        if (s != gr_.st_.EPSILON_) {
            result.insert(s);
        }
    }

    if (fii.find(gr_.st_.EPSILON_) == fii.cend()) {
        return;
    }
    First(std::span<const std::string>(rule.begin() + 1, rule.end()), result);
}

// Least fixed point
void LL1Parser::ComputeFirstSets() {
    // Init all FIRST to empty
    for (const auto& [nonTerminal, _] : gr_.g_) {
        first_sets[nonTerminal] = {};
    }

    bool changed;
    do {
        auto old_first_sets = first_sets; // Copy current state

        for (const auto& [nonTerminal, productions] : gr_.g_) {
            for (const auto& prod : productions) {
                std::unordered_set<std::string> tempFirst;
                First(prod, tempFirst);

                if (tempFirst.find(gr_.st_.EOL_) != tempFirst.end()) {
                    tempFirst.erase(gr_.st_.EOL_);
                    tempFirst.insert(gr_.st_.EPSILON_);
                }

                auto& current_set = first_sets[nonTerminal];
                current_set.insert(tempFirst.begin(), tempFirst.end());
            }
        }

        // Until all remain the same
        changed = (old_first_sets != first_sets);

    } while (changed);
}

std::unordered_set<std::string> LL1Parser::Follow(const std::string& arg) {
    std::unordered_set<std::string> next_symbols;
    std::unordered_set<std::string> visited;
    if (arg == gr_.axiom_) {
        return {gr_.st_.EOL_};
    }
    FollowUtil(arg, visited, next_symbols);
    if (next_symbols.find(gr_.st_.EPSILON_) != next_symbols.end()) {
        next_symbols.erase(gr_.st_.EPSILON_);
    }
    return next_symbols;
}

void LL1Parser::FollowUtil(const std::string&               arg,
                           std::unordered_set<std::string>& visited,
                           std::unordered_set<std::string>& next_symbols) {
    if (visited.find(arg) != visited.cend()) {
        return;
    }
    visited.insert(arg);
    std::vector<std::pair<const std::string, production>> rules{
        gr_.FilterRulesByConsequent(arg)};
    for (const std::pair<const std::string, production>& rule : rules) {
        // Next must be applied to all Arg symbols, for example
        // if arg: B; A -> BbBCB, next is applied three times
        auto it = rule.second.cbegin();
        while ((it = std::find(it, rule.second.cend(), arg)) !=
               rule.second.cend()) {
            auto next_it = std::next(it);
            if (next_it == rule.second.cend()) {
                FollowUtil(rule.first, visited, next_symbols);
            } else {
                if (*next_it == gr_.st_.EOL_) {
                    next_symbols.insert(gr_.st_.EOL_);
                } else {
                    First(std::span<const std::string>(next_it,
                                                       rule.second.cend()),
                          next_symbols);
                    if (next_symbols.find(gr_.st_.EPSILON_) !=
                        next_symbols.end()) {
                        next_symbols.erase(gr_.st_.EPSILON_);
                        FollowUtil(rule.first, visited, next_symbols);
                    }
                }
            }
            it = std::next(it);
        }
    }
}

std::unordered_set<std::string>
LL1Parser::PredictionSymbols(const std::string&              antecedent,
                             const std::vector<std::string>& consequent) {
    std::unordered_set<std::string> hd{};
    First({consequent}, hd);
    if (hd.find(gr_.st_.EPSILON_) == hd.end()) {
        return hd;
    }
    hd.erase(gr_.st_.EPSILON_);
    hd.merge(Follow(antecedent));
    return hd;
}

void LL1Parser::PrintTable() {
    using namespace tabulate;
    Table table;

    Table::Row_t                          headers = {"Non-terminal"};
    std::unordered_map<std::string, bool> columns;

    for (const auto& outerPair : ll1_t_) {
        for (const auto& innerPair : outerPair.second) {
            columns[innerPair.first] = true;
        }
    }

    for (const auto& col : columns) {
        headers.push_back(col.first);
    }

    auto& header_row = table.add_row(headers);
    header_row.format()
        .font_align(FontAlign::center)
        .font_color(Color::yellow)
        .font_style({FontStyle::bold});

    for (const auto& outerPair : ll1_t_) {
        const std::string& nonTerminal = outerPair.first;
        Table::Row_t       row_data    = {nonTerminal};

        for (const auto& col : columns) {
            auto innerIt = outerPair.second.find(col.first);
            if (innerIt != outerPair.second.end()) {
                std::string cell_content;
                for (const auto& prod : innerIt->second) {
                    cell_content += "[ ";
                    for (const std::string& elem : prod) {
                        cell_content += elem + " ";
                    }
                    cell_content += "] ";
                }
                row_data.push_back(cell_content);
            } else {
                row_data.push_back("-");
            }
        }
        table.add_row(row_data);
    }

    table[0].format().font_color(Color::cyan).font_style({FontStyle::bold});
    for (size_t i = 1; i < table.size(); ++i) {
        for (size_t j = 1; j < table[i].size(); ++j) {
            if (table[i][j].get_text().find("] [") != std::string::npos) {
                table[i][j].format().font_color(Color::red);
            }
        }
    }
    table.format().font_align(FontAlign::center);
    table.column(0).format().font_color(Color::cyan);
    std::cout << table << std::endl;
}
