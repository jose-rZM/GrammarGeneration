#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_set>
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
                items.insert({rule.first, production, i, gr_.st_.EPSILON_,
                              gr_.st_.EOL_});
        }
    }
    return items;
}

void SLR1Parser::DebugStates() const {
    tabulate::Table        table;
    tabulate::Table::Row_t header = {"State ID", "Items"};
    table.add_row(header);

    for (size_t state = 0; state < states_.size(); ++state) {
        tabulate::Table::Row_t row;
        const auto             currentIt = std::find_if(
            states_.begin(), states_.end(),
            [state](const auto& st) -> bool { return st.id_ == state; });
        row.push_back(std::to_string(state));
        std::string str = "";
        for (const auto& item : currentIt->items_) {
            str += item.ToString();
            str += "\n";
        }
        row.push_back(str);
        table.add_row(row);
    }
    table.column(0)
        .format()
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::cyan);
    table.row(0).format().font_align(tabulate::FontAlign::center);
    table.column(1).format().font_color(tabulate::Color::yellow);
    std::cout << table << "\n";
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
                rule += action.item->antecedent_ + " -> ";
                for (const auto& sym : action.item->consequent_) {
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

void SLR1Parser::MakeInitialState() {
    state initial;
    initial.id_ = 0;
    auto axiom  = gr_.g_.at(gr_.axiom_);
    // the axiom must be unique
    initial.items_.insert(
        {gr_.axiom_, axiom[0], gr_.st_.EPSILON_, gr_.st_.EOL_});
    Closure(initial.items_);
    states_.insert(initial);
}

bool SLR1Parser::SolveLRConflicts(const state& st) {
    for (const Lr0Item& item : st.items_) {
        if (item.IsComplete()) {
            // Regla 3: Si el ítem es del axioma, ACCEPT en EOL
            if (item.antecedent_ == gr_.axiom_) {
                actions_[st.id_][gr_.st_.EOL_] = {nullptr, Action::Accept};
            } else {
                // Regla 2: Si el ítem es completo, REDUCE en FOLLOW(A)
                std::unordered_set<std::string> follows =
                    Follow(item.antecedent_);
                for (const std::string& sym : follows) {
                    auto it = actions_[st.id_].find(sym);
                    if (it != actions_[st.id_].end()) {
                        // Si ya hay un Reduce, comparar las reglas.
                        // REDUCE/REDUCE si reglas distintas
                        if (it->second.action == Action::Reduce) {
                            if (!(it->second.item->antecedent_ ==
                                      item.antecedent_ &&
                                  it->second.item->consequent_ ==
                                      item.consequent_)) {
                                return false;
                            }
                        } else {
                            return false; // SHIFT/REDUCE
                        }
                    }
                    actions_[st.id_][sym] = {&item, Action::Reduce};
                }
            }
        } else {
            // Regla 1: Si hay un terminal después del punto, hacemos SHIFT
            std::string nextToDot = item.NextToDot();
            if (gr_.st_.IsTerminal(nextToDot)) {
                auto it = actions_[st.id_].find(nextToDot);
                if (it != actions_[st.id_].end()) {
                    // Si hay una acción previa, hay conflicto si es REDUCE
                    if (it->second.action == Action::Reduce) {
                        return false;
                    }
                    // Si ya hay un SHIFT en esa celda, no hay conflicto (varios
                    // SHIFT están permitidos)
                }
                actions_[st.id_][nextToDot] = {nullptr, Action::Shift};
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
            [current](const state& st) -> bool { return st.id_ == current; });
        if (it == states_.end()) {
            break;
        }
        const state& qi = *it;
        std::for_each(qi.items_.begin(), qi.items_.end(),
                      [&](const Lr0Item& item) -> void {
                          std::string next = item.NextToDot();
                          if (next != gr_.st_.EPSILON_ &&
                              next != gr_.st_.EOL_) {
                              nextSymbols.insert(next);
                          }
                      });
        for (const std::string& symbol : nextSymbols) {
            state newState;
            newState.id_ = i;
            for (const auto& item : qi.items_) {
                if (item.NextToDot() == symbol) {
                    Lr0Item newItem = item;
                    newItem.AdvanceDot();
                    newState.items_.insert(newItem);
                }
            }

            Closure(newState.items_);
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
                    transitions_[current].insert({symbol, result.first->id_});
                } else {
                    std::map<std::string, unsigned int> column;
                    column.insert({symbol, result.first->id_});
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
        std::string next = item.NextToDot();
        if (next == gr_.st_.EPSILON_) {
            continue;
        }
        if (!gr_.st_.IsTerminal(next) &&
            std::find(visited.cbegin(), visited.cend(), next) ==
                visited.cend()) {
            const std::vector<production>& rules = gr_.g_.at(next);
            std::for_each(rules.begin(), rules.end(),
                          [&](const auto& rule) -> void {
                              newItems.insert({item.NextToDot(), rule,
                                               gr_.st_.EPSILON_, gr_.st_.EOL_});
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
void SLR1Parser::ComputeFirstSets() {
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
