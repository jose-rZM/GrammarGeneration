#include "grammar_factory.hpp"
#include <algorithm>
#include <iostream>
#include <queue>
#include <random>

void GrammarFactory::Init() {
    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "b", "A"}, {"a"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "b", "A"}, {"a", "b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A", "b"}, {"EPSILON"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"A", "a"}, {"EPSILON"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A"}, {"EPSILON"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A", "c"}, {"b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A", "a"}, {"b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"A", "a"}, {"b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"b", "A"}, {"a"}}}});
}

Grammar GrammarFactory::PickOne(int level) {
    switch (level) {
    case 1:
        return Lv1();
        break;
    case 2:
        return Lv2();
        break;
    case 3:
        return Lv3();
        break;
    default:
        return Grammar();
        break;
    }
}

Grammar GrammarFactory::Lv1() {
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

Grammar GrammarFactory::Lv2() {
    return Grammar(CreateLv2Item().g_);
}

Grammar GrammarFactory::Lv3() {
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

GrammarFactory::FactoryItem GrammarFactory::CreateLv2Item() {
    // STEP 1 Choose a random base grammar ----------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           base = items.at(dist(gen));
    // -----------------------------------------------------

    // STEP 2 Choose a random cmb grammar such that base != cmb
    // ------------------------------
    FactoryItem cmb = items.at(dist(gen));
    while (base.g_ == cmb.g_) {
        cmb = items.at(dist(gen));
    }
    // -----------------------------------------------------

    // STEP 3 Change non terminals in cmb to B
    // --------------------------------------------
    std::unordered_map<std::string, std::vector<production>>
        cmb_updated_grammar;
    cmb.st_.non_terminals_.insert("B");
    for (auto& [nt, prods] : cmb.g_) {
        std::string new_nt = "B";
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (!cmb.st_.IsTerminal(symbol)) {
                    symbol = "B";
                    cmb.st_.non_terminals_.erase(symbol);
                }
            }
        }
        cmb_updated_grammar["B"] = prods;
    }
    cmb.g_ = std::move(cmb_updated_grammar);
    // -----------------------------------------------------

    // STEP 4 Change one base terminal to another that is not in cmb
    std::unordered_set<std::string> cmb_terminals = cmb.st_.terminals_wtho_eol_;
    std::unordered_set<std::string> terminal_alphabet_set(
        terminal_alphabet_.begin(), terminal_alphabet_.end());

    for (const std::string& terminal : cmb_terminals) {
        terminal_alphabet_set.erase(terminal);
    }

    std::uniform_int_distribution<size_t> terminal_dist(
        0, terminal_alphabet_set.size() - 1);
    std::vector<std::string> remaining_terminals(terminal_alphabet_set.begin(),
                                                 terminal_alphabet_set.end());
    std::string new_terminal = remaining_terminals[terminal_dist(gen)];

    std::uniform_int_distribution<size_t> base_terminal_dist(
        0, base.st_.terminals_wtho_eol_.size() - 1);
    std::vector<std::string> base_terminals(
        base.st_.terminals_wtho_eol_.begin(),
        base.st_.terminals_wtho_eol_.end());
    std::string terminal_to_replace =
        base_terminals.at(base_terminal_dist(gen));

    for (auto& [nt, prods] : base.g_) {
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (symbol == terminal_to_replace) {
                    symbol = new_terminal;
                }
            }
        }
    }
    base.st_.terminals_wtho_eol_.erase(terminal_to_replace);
    base.st_.terminals_wtho_eol_.insert(new_terminal);
    base_terminal_dist = std::uniform_int_distribution<size_t>(
        0, base.st_.terminals_wtho_eol_.size() - 1);
    // -----------------------------------------------------

    // STEP 5 Change one random terminal -> terminal B
    terminal_to_replace = *std::next(base.st_.terminals_wtho_eol_.begin(),
                                     base_terminal_dist(gen));
    for (auto& [nt, prods] : base.g_) {
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (symbol == terminal_to_replace) {
                    symbol = "B";
                }
            }
        }
    }

    std::unordered_map<std::string, std::vector<production>> combined_grammar =
        base.g_;
    for (auto& [nt, prods] : cmb.g_) {
        combined_grammar[nt].insert(combined_grammar[nt].end(), prods.begin(),
                                    prods.end());
    }

    return FactoryItem(combined_grammar);
}

bool GrammarFactory::HasUnreachableSymbols(Grammar& grammar) {
    std::unordered_set<std::string> reachable;
    std::queue<std::string>         pending;

    pending.push(grammar.axiom_);
    reachable.insert(grammar.axiom_);

    while (!pending.empty()) {
        std::string current = pending.front();
        pending.pop();

        auto it = grammar.g_.find(current);
        if (it != grammar.g_.end()) {
            for (const auto& production : it->second) {
                for (const auto& symbol : production) {
                    if (!grammar.st_.IsTerminal(symbol) &&
                        reachable.find(symbol) == reachable.end()) {
                        reachable.insert(symbol);
                        pending.push(symbol);
                    }
                }
            }
        }
    }

    for (const auto& nt : grammar.st_.non_terminals_) {
        if (reachable.find(nt) == reachable.end()) {
            return true;
        }
    }
    return false;
}

bool GrammarFactory::IsInfinite(Grammar& grammar) {
    std::unordered_set<std::string> generating;
    bool                            changed = true;

    while (changed) {
        changed = false;
        for (const auto& [nt, productions] : grammar.g_) {
            if (generating.find(nt) != generating.end()) {
                continue;
            }
            for (const auto& prod : productions) {
                bool all_generating = true;
                for (const auto& symbol : prod) {
                    if (!grammar.st_.IsTerminal(symbol) &&
                        generating.find(symbol) == generating.end()) {
                        all_generating = false;
                        break;
                    }
                }
                if (all_generating) {
                    generating.insert(nt);
                    changed = true;
                    break;
                }
            }
        }
    }
    // Counterexample:  S -> A; A -> B A c | e; B -> B a | B. Axiom can derive
    // into a terminal string (A -> e) return generating.find(grammar.axiom_) ==
    // generating.end();
    return generating != grammar.st_.non_terminals_;
}

bool GrammarFactory::HasDirectLeftRecursion(Grammar& grammar) {
    for (const auto& [nt, prods] : grammar.g_) {
        for (const auto& prod : prods) {
            if (nt == prod[0]) {
                return true;
            }
        }
    }
    return false;
}

GrammarFactory::FactoryItem::FactoryItem(
    const std::unordered_map<std::string, std::vector<production>>& grammar) {
    for (const auto& [nt, prods] : grammar) {
        st_.PutSymbol(nt, false);
        for (const auto& prod : prods) {
            for (const std::string& symbol : prod) {
                if (symbol == "EPSILON") {
                    st_.PutSymbol(symbol, true);
                    continue;
                } else if (std::islower(symbol[0])) {
                    st_.PutSymbol(symbol, true);
                }
            }
        }
    }
    g_ = (grammar);
}

bool GrammarFactory::FactoryItem::HasEmptyProduction(
    const std::string& antecedent) {
    auto& rules = g_.at(antecedent);
    return std::find_if(rules.cbegin(), rules.cend(), [&](const auto& rule) {
               return rule[0] == "EPSILON";
           }) != rules.cend();
}

void GrammarFactory::FactoryItem::Debug() {
    std::cout << "Grammar:\n";
    for (const auto& entry : g_) {
        std::cout << entry.first << " -> ";
        for (const std::vector<std::string>& prod : entry.second) {
            for (const std::string& symbol : prod) {
                std::cout << symbol << " ";
            }
            std::cout << "| ";
        }
        std::cout << "\n";
    }
}