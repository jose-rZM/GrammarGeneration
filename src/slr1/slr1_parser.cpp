#include <algorithm>
#include <format>
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

std::unordered_set<Lr0Item> SLR1Parser::AllItems() const {
    std::unordered_set<Lr0Item> items;
    for (const auto& [lhs, productions] : gr_.g_) {
        for (const auto& production : productions) {
            for (unsigned int i = 0; i <= production.size(); ++i)
                items.emplace(lhs, production, i, gr_.st_.EPSILON_,
                              gr_.st_.EOL_);
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
        const auto             currentIt =
            std::ranges::find_if(states_, [state](const auto& st) -> bool {
                return st.id_ == state;
            });
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
            std::string cell = "-";

            if (const bool is_terminal = gr_.st_.IsTerminal(symbol);
                !is_terminal) {
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
                                    cell = std::format("S{}", shift_it->second);
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
    initial.items_.emplace(gr_.axiom_, axiom[0], gr_.st_.EPSILON_,
                           gr_.st_.EOL_);
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
                    if (auto it = actions_[st.id_].find(sym);
                        it != actions_[st.id_].end()) {
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
                if (auto it = actions_[st.id_].find(nextToDot);
                    it != actions_[st.id_].end()) {
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
    ComputeFollowSets();
    MakeInitialState();
    std::queue<unsigned int> pending;
    pending.push(0);
    unsigned int current = 0;
    size_t       i       = 1;

    do {
        std::unordered_set<std::string> nextSymbols;
        current = pending.front();
        pending.pop();
        auto it = std::ranges::find_if(
            states_, [current](const state& st) { return st.id_ == current; });
        if (it == states_.end()) {
            break;
        }
        const state& qi = *it;

        std::ranges::for_each(qi.items_, [&](const Lr0Item& item) {
            std::string next = item.NextToDot();
            if (next != gr_.st_.EPSILON_) {
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
            auto [iterator, inserted] = states_.insert(newState);
            std::map<std::string, unsigned int> column;

            if (inserted) {
                pending.push(i);
                if (transitions_.contains(current)) {
                    transitions_[current].try_emplace(symbol, i);
                } else {
                    column.try_emplace(symbol, i);
                    transitions_.try_emplace(current, std::move(column));
                }
                ++i;
            } else {
                if (transitions_.contains(current)) {
                    transitions_[current].try_emplace(symbol, iterator->id_);
                } else {
                    column.try_emplace(symbol, iterator->id_);
                    transitions_.try_emplace(current, std::move(column));
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
    return std::ranges::all_of(
        states_, [this](const state& st) { return SolveLRConflicts(st); });
}

void SLR1Parser::Closure(std::unordered_set<Lr0Item>& items) {
    std::unordered_set<std::string> visited;
    ClosureUtil(items, items.size(), visited);
}

void SLR1Parser::ClosureUtil(std::unordered_set<Lr0Item>&     items,
                             std::size_t                      size,
                             std::unordered_set<std::string>& visited) {
    std::unordered_set<Lr0Item> newItems;

    for (const auto& item : items) {
        std::string next = item.NextToDot();
        if (next == gr_.st_.EPSILON_) {
            continue;
        }
        if (!gr_.st_.IsTerminal(next) && !visited.contains(next)) {
            const std::vector<production>& rules = gr_.g_.at(next);
            std::ranges::for_each(rules, [&](const auto& rule) {
                newItems.insert(
                    {item.NextToDot(), rule, gr_.st_.EPSILON_, gr_.st_.EOL_});
            });
            visited.insert(next);
        }
    }
    items.insert(newItems.begin(), newItems.end());
    if (size != items.size())
        ClosureUtil(items, items.size(), visited);
}

std::unordered_set<Lr0Item>
SLR1Parser::Delta(const std::unordered_set<Lr0Item>& items,
                  const std::string&                 str) {
    if (str == gr_.st_.EPSILON_) {
        return {}; // DELTA(I, EPSILON) = empty
    }
    std::vector<Lr0Item> filtered;
    std::ranges::for_each(items, [&](const Lr0Item& item) -> void {
        std::string next = item.NextToDot();
        if (next == str) {
            filtered.push_back(item);
        }
    });
    if (filtered.empty()) {
        return {};
    } else {
        std::unordered_set<Lr0Item> delta_items;
        delta_items.reserve(filtered.size());
        for (Lr0Item& lr : filtered) {
            lr.AdvanceDot();
            delta_items.insert(lr);
        }
        Closure(delta_items);
        return delta_items;
    }
}

void SLR1Parser::First(std::span<const std::string>     rule,
                       std::unordered_set<std::string>& result) {
    if (rule.empty() || (rule.size() == 1 && rule[0] == gr_.st_.EPSILON_)) {
        result.insert(gr_.st_.EPSILON_);
        return;
    }

    if (rule.size() > 1 && rule[0] == gr_.st_.EPSILON_) {
        First(std::span<const std::string>(rule.begin() + 1, rule.end()),
              result);
    } else {

        if (gr_.st_.IsTerminal(rule[0])) {
            // EOL cannot be in first sets, if we reach EOL it means that the
            // axiom is nullable, so epsilon is included instead
            if (rule[0] == gr_.st_.EOL_) {
                result.insert(gr_.st_.EPSILON_);
                return;
            }
            result.insert(rule[0]);
            return;
        }

        const std::unordered_set<std::string>& fii = first_sets_[rule[0]];
        for (const auto& s : fii) {
            if (s != gr_.st_.EPSILON_) {
                result.insert(s);
            }
        }

        if (!fii.contains(gr_.st_.EPSILON_)) {
            return;
        }
        First(std::span<const std::string>(rule.begin() + 1, rule.end()),
              result);
    }
}

// Least fixed point
void SLR1Parser::ComputeFirstSets() {
    // Init all FIRST to empty
    for (const auto& [nonTerminal, _] : gr_.g_) {
        first_sets_[nonTerminal] = {};
    }

    bool changed;
    do {
        auto old_first_sets = first_sets_; // Copy current state

        for (const auto& [nonTerminal, productions] : gr_.g_) {
            for (const auto& prod : productions) {
                std::unordered_set<std::string> tempFirst;
                First(prod, tempFirst);

                if (tempFirst.contains(gr_.st_.EOL_)) {
                    tempFirst.erase(gr_.st_.EOL_);
                    tempFirst.insert(gr_.st_.EPSILON_);
                }

                auto& current_set = first_sets_[nonTerminal];
                current_set.insert(tempFirst.begin(), tempFirst.end());
            }
        }

        // Until all remain the same
        changed = (old_first_sets != first_sets_);

    } while (changed);
}

void SLR1Parser::ComputeFollowSets() {
    for (const auto& [nt, _] : gr_.g_) {
        follow_sets_[nt] = {};
    }
    follow_sets_[gr_.axiom_].insert(gr_.st_.EOL_);

    bool changed;
    do {
        changed = false;
        for (const auto& [lhs, productions] : gr_.g_) {
            for (const production& rhs : productions) {
                for (size_t i = 0; i < rhs.size(); ++i) {
                    const std::string& symbol = rhs[i];
                    if (!gr_.st_.IsTerminal(symbol)) {
                        changed |= UpdateFollow(symbol, lhs, rhs, i);
                    }
                }
            }
        }
    } while (changed);
}

bool SLR1Parser::UpdateFollow(const std::string& symbol, const std::string& lhs,
                              const production& rhs, size_t i) {
    bool changed = false;

    std::unordered_set<std::string> first_remaining;
    if (i + 1 < rhs.size()) {
        First(std::span<const std::string>(rhs.begin() + i + 1, rhs.end()),
              first_remaining);
    } else {
        first_remaining.insert(gr_.st_.EPSILON_);
    }

    // Add FIRST(β) \ {ε}
    for (const auto& terminal : first_remaining) {
        if (terminal != gr_.st_.EPSILON_) {
            changed |= follow_sets_[symbol].insert(terminal).second;
        }
    }

    // If FIRST(β) contains ε, add FOLLOW(lhs)
    if (first_remaining.contains(gr_.st_.EPSILON_)) {
        for (const auto& terminal : follow_sets_[lhs]) {
            changed |= follow_sets_[symbol].insert(terminal).second;
        }
    }

    return changed;
}

std::unordered_set<std::string> SLR1Parser::Follow(const std::string& arg) {
    if (!follow_sets_.contains(arg)) {
        return {};
    }
    return follow_sets_.at(arg);
}
