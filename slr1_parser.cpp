#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "grammar.hpp"
#include "slr1_parser.hpp"
#include "symbol_table.hpp"
#include "tabulate.hpp"

SLR1Parser::SLR1Parser(Grammar gr) : gr_(std::move(gr)) {}

std::unordered_set<Lr0Item> SLR1Parser::allItems() const {
    std::unordered_set<Lr0Item> items;
    for (const auto& rule : gr_.g_) {
        for (const auto& production : rule.second) {
            for (unsigned int i = 0; i <= production.size(); ++i)
                items.insert({rule.first, production, i});
        }
    }
    return items;
}

void SLR1Parser::DebugStates() const {
    for (const auto& s : states_) {
        std::cout << "State ID: " << s.id << std::endl;
        std::cout << "Items:" << std::endl;
        for (const auto& item : s.items) {
            std::cout << "\t";
            item.printItem();
            std::cout << std::endl;
        }
        std::cout << "-----------------------------" << std::endl;
    }
}

void SLR1Parser::DebugActions() {
    std::vector<std::string> columns;
    columns.reserve(gr_.st_.terminals_.size() + gr_.st_.non_terminals_.size());
    tabulate::Table        table;
    tabulate::Table::Row_t header = {"State"};
    for (const auto& s : gr_.st_.terminals_) {
        if (s == gr_.st_.EPSILON_) {
            continue;
        }
        columns.push_back(s);
    }
    columns.insert(columns.end(), gr_.st_.non_terminals_.begin(),
                   gr_.st_.non_terminals_.end());
    header.insert(header.end(), columns.begin(), columns.end());
    table.add_row(header);

    for (unsigned state = 0; state < states_.size(); ++state) {
        tabulate::Table::Row_t row_data{std::to_string(state)};

        const auto  action_entry = actions_.find(state);
        const auto  trans_entry  = transitions_.find(state);
        const auto& transitions  = trans_entry->second;
        for (const auto& symbol : columns) {
            std::string cell        = "-";
            const bool  is_terminal = gr_.st_.IsTerminal(symbol);

            if (!is_terminal) {
                if (trans_entry != transitions_.end()) {
                    const auto it = transitions.find(symbol);
                    if (it != transitions.end()) {
                        cell = std::to_string(it->second);
                    }
                }
            } else {
                if (action_entry != actions_.end()) {
                    const auto action_it = action_entry->second.find(symbol);
                    if (action_it != action_entry->second.end()) {
                        switch (action_it->second.action) {
                        case Action::Accept:
                            cell = "A";
                            break;
                        case Action::Reduce:
                            cell = "R";
                            break;
                        case Action::Shift:
                            if (trans_entry != transitions_.end()) {
                                const auto shift_it = transitions.find(symbol);
                                if (shift_it != transitions.end()) {
                                    cell =
                                        "S" + std::to_string(shift_it->second);
                                }
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            row_data.push_back(cell);
        }
        table.add_row(row_data);
    }
    table.format().font_align(tabulate::FontAlign::center);
    table.column(0).format().font_color(tabulate::Color::cyan);
    table.row(0).format().font_color(tabulate::Color::magenta);
    std::cout << table << std::endl;

    tabulate::Table reduce_table;
    reduce_table.add_row({"State", "Symbol", "Production Rule"});

    const auto state_color  = tabulate::Color::cyan;
    const auto symbol_color = tabulate::Color::yellow;
    const auto rule_color   = tabulate::Color::magenta;

    for (const auto& [state, actions] : actions_) {
        for (const auto& [symbol, action] : actions) {
            if (action.action == Action::Reduce) {
                tabulate::Table::Row_t row;
                std::string            rule;
                rule += action.item->antecedent + " -> ";
                for (const auto& sym : action.item->consequent) {
                    rule += sym + " ";
                }
                row.push_back(std::to_string(state));
                row.push_back(symbol);
                row.push_back(rule);
                reduce_table.add_row(row);
            }
        }
    }
    reduce_table.format().font_align(tabulate::FontAlign::center);
    reduce_table.column(0).format().font_color(state_color);
    reduce_table.column(1).format().font_color(symbol_color);
    reduce_table.column(2).format().font_color(rule_color);

    std::cout << "\n\n";
    std::cout << "Reduce Actions:" << std::endl;
    std::cout << reduce_table << std::endl;
}

void SLR1Parser::DebugTable() const {
    std::map<std::string, bool> columns;

    for (const auto& row : transitions_) {
        for (const auto& cell : row.second) {
            columns[cell.first] = true;
        }
    }

    std::cout << std::setw(10) << "TRANSITIONS |";
    for (const auto& col : columns) {
        std::cout << std::setw(10) << col.first << " |";
    }
    std::cout << std::endl;

    std::cout << std::string(10 + (columns.size() * 13), '-') << std::endl;

    for (unsigned int state = 0; state < states_.size(); ++state) {
        std::cout << std::setw(10) << state << " |";

        auto rowIt = transitions_.find(state);
        if (rowIt != transitions_.end() && !rowIt->second.empty()) {
            for (const auto& col : columns) {
                auto cellIt = rowIt->second.find(col.first);
                if (cellIt != rowIt->second.end()) {
                    std::cout << std::setw(10) << cellIt->second << " |";
                } else {
                    std::cout << std::setw(10) << "-" << " |";
                }
            }
        } else {
            for (const auto& _ : columns) {
                std::cout << std::setw(10) << "-" << " |";
            }
        }
        std::cout << std::endl;
    }
}

void SLR1Parser::MakeInitialState() {
    state initial;
    initial.id = 0;
    auto axiom = gr_.g_.at(gr_.axiom_);
    // the axiom must be unique
    initial.items.insert({gr_.axiom_, axiom[0], 0});
    Closure(initial.items);
    states_.insert(initial);
}

bool SLR1Parser::SolveLRConflicts(const state& st) {
    for (const Lr0Item& item : st.items) {
        if (item.isComplete()) {
            // Regla 3: Si el ítem es del axioma, ACCEPT en EOL
            if (item.antecedent == gr_.axiom_) {
                actions_[st.id][gr_.st_.EOL_] = {nullptr, Action::Accept};
            } else {
                // Regla 2: Si el ítem es completo, REDUCE en FOLLOW(A)
                std::unordered_set<std::string> follows =
                    Follow(item.antecedent);
                for (const std::string& sym : follows) {
                    auto it = actions_[st.id].find(sym);
                    if (it != actions_[st.id].end()) {
                        // Si ya hay un Reduce, comparar las reglas.
                        // REDUCE/REDUCE si reglas distintas
                        if (it->second.action == Action::Reduce) {
                            if (!(it->second.item->antecedent ==
                                      item.antecedent &&
                                  it->second.item->consequent ==
                                      item.consequent)) {
                                return false;
                            }
                        } else {
                            return false; // SHIFT/REDUCE
                        }
                    }
                    actions_[st.id][sym] = {&item, Action::Reduce};
                }
            }
        } else {
            // Regla 1: Si hay un terminal después del punto, hacemos SHIFT
            std::string nextToDot = item.nextToDot();
            if (gr_.st_.IsTerminal(nextToDot)) {
                auto it = actions_[st.id].find(nextToDot);
                if (it != actions_[st.id].end()) {
                    // Si hay una acción previa, hay conflicto si es REDUCE
                    if (it->second.action == Action::Reduce) {
                        return false;
                    }
                    // Si ya hay un SHIFT en esa celda, no hay conflicto (varios
                    // SHIFT están permitidos)
                }
                actions_[st.id][nextToDot] = {nullptr, Action::Shift};
            }
        }
    }
    return true;
}

bool SLR1Parser::MakeParser() {
    ComputeFirstSets();
    MakeInitialState();
    std::queue<unsigned int> pending;
    pending.push(0);
    unsigned int current = 0;
    size_t       i       = 1;

    do {
        std::unordered_set<std::string> nextSymbols;
        current = pending.front();
        pending.pop();
        auto it = std::find_if(
            states_.begin(), states_.end(),
            [current](const state& st) -> bool { return st.id == current; });
        if (it == states_.end()) {
            break;
        }
        const state& qi = *it;
        std::for_each(
            qi.items.begin(), qi.items.end(), [&](const Lr0Item& item) -> void {
                std::string next = item.nextToDot();
                if (next != gr_.st_.EPSILON_ && next != gr_.st_.EOL_) {
                    nextSymbols.insert(next);
                }
            });
        for (const std::string& symbol : nextSymbols) {
            state newState;
            newState.id = i;
            for (const auto& item : qi.items) {
                if (item.nextToDot() == symbol) {
                    Lr0Item newItem = item;
                    newItem.advanceDot();
                    newState.items.insert(newItem);
                }
            }

            Closure(newState.items);
            auto result = states_.insert(newState);
            std::map<std::string, unsigned int> column;

            if (result.second) {
                pending.push(i);
                if (transitions_.find(current) != transitions_.end()) {
                    transitions_[current].insert({symbol, i});
                } else {
                    std::map<std::string, unsigned int> column;
                    column.insert({symbol, i});
                    transitions_.insert({current, column});
                }
                ++i;
            } else {
                if (transitions_.find(current) != transitions_.end()) {
                    transitions_[current].insert({symbol, result.first->id});
                } else {
                    std::map<std::string, unsigned int> column;
                    column.insert({symbol, result.first->id});
                    transitions_.insert({current, column});
                }
            }
        }
        current++;
    } while (!pending.empty());
    for (const state& st : states_) {
        if (!SolveLRConflicts(st)) {
            return false;
        }
    }
    return true;
}

void SLR1Parser::Closure(std::unordered_set<Lr0Item>& items) {
    std::unordered_set<std::string> visited;
    ClosureUtil(items, items.size(), visited);
}

void SLR1Parser::ClosureUtil(std::unordered_set<Lr0Item>&     items,
                             unsigned int                     size,
                             std::unordered_set<std::string>& visited) {
    std::unordered_set<Lr0Item> newItems;

    for (const auto& item : items) {
        std::string next = item.nextToDot();
        if (next == gr_.st_.EPSILON_) {
            continue;
        }
        if (!gr_.st_.IsTerminal(next) &&
            std::find(visited.cbegin(), visited.cend(), next) ==
                visited.cend()) {
            const std::vector<production>& rules = gr_.g_.at(next);
            std::for_each(rules.begin(), rules.end(),
                          [&](const auto& rule) -> void {
                              newItems.insert({item.nextToDot(), rule});
                          });
            visited.insert(next);
        }
    }
    items.insert(newItems.begin(), newItems.end());
    if (size != items.size())
        ClosureUtil(items, items.size(), visited);
}

void SLR1Parser::First(std::span<const std::string>     rule,
                       std::unordered_set<std::string>& result) {
    if (rule.size() == 1 && rule[0] == gr_.st_.EPSILON_) {
        result.insert(gr_.st_.EPSILON_);
    }
    std::unordered_set<std::string> ret;
    size_t                          i{0};
    for (const std::string& symbol : rule) {
        if (gr_.st_.IsTerminal(symbol)) {
            result.insert(symbol);
            break;
        } else {
            const std::unordered_set<std::string>& fi = first_sets[symbol];
            result.insert(fi.begin(), fi.end());
            result.erase(gr_.st_.EPSILON_);
            if (fi.find(gr_.st_.EPSILON_) == fi.cend()) {
                break;
            }
            ++i;
        }
    }

    if (i == rule.size()) {
        result.insert(gr_.st_.EPSILON_);
    }
}

void SLR1Parser::ComputeFirstSets() {
    for (const auto& rule : gr_.g_) {
        first_sets[rule.first] = {};
    }
    bool changed{true};
    while (changed) {
        changed = false;
        for (const auto& rule : gr_.g_) {
            const std::string& nonTerminal = rule.first;
            std::size_t        beforeSize  = first_sets[nonTerminal].size();
            for (const auto& prod : rule.second) {
                First(prod, first_sets[nonTerminal]);
            }
            if (first_sets[nonTerminal].size() > beforeSize) {
                changed = true;
            }
        }
    }
    first_sets[gr_.axiom_].erase(gr_.st_.EOL_);
}

std::unordered_set<std::string> SLR1Parser::Follow(const std::string& arg) {
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

void SLR1Parser::FollowUtil(const std::string&               arg,
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
                First(std::span<const std::string>(next_it, rule.second.cend()),
                      next_symbols);
                if (next_symbols.find(gr_.st_.EPSILON_) != next_symbols.end()) {
                    next_symbols.erase(gr_.st_.EPSILON_);
                    FollowUtil(rule.first, visited, next_symbols);
                }
            }
            it = std::next(it);
        }
    }
}
