#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
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
    case 4:
        return Lv4();
        break;
    case 5:
        return Lv5();
        break;
    default:
        return Lv6();
        break;
    }
}

Grammar GrammarFactory::GenLL1Grammar(int level) {
    Grammar   gr = PickOne(level);
    LL1Parser ll1(gr);
    while (IsInfinite(gr) || HasUnreachableSymbols(gr) ||
           HasDirectLeftRecursion(gr) || !ll1.CreateLL1Table()) {
        RemoveLeftRecursion(gr);
        ll1 = LL1Parser(gr);
        if (ll1.CreateLL1Table()) {
            break;
        }
        gr = PickOne(level);
    }
    return gr;
}

Grammar GrammarFactory::GenSLR1Grammar(int level) {
    Grammar    gr = PickOne(level);
    SLR1Parser slr1(gr);

    while (IsInfinite(gr) || HasUnreachableSymbols(gr) || !slr1.MakeParser()) {
        gr   = PickOne(level);
        slr1 = SLR1Parser(gr);
    }
    return gr;
}

void GrammarFactory::SanityChecks(Grammar& gr) {
    std::cout << "Sanity check (Is Infinite?) : " << IsInfinite(gr) << "\n";
    std::cout << "Sanity check (Has Unreachable Symbols?) : "
              << HasUnreachableSymbols(gr) << "\n";
    std::cout << "Sanity check (Has Direct Left Recursion?) : "
              << HasDirectLeftRecursion(gr) << "\n";
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
    // STEP 1 Build a random LV2 base grammar ------------------------------
    FactoryItem base = CreateLv2Item();

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    // STEP 3 Change non terminals in cmb to C ---------------------------
    std::unordered_map<std::string, std::vector<production>>
        cmb_updated_grammar;
    cmb.st_.non_terminals_.insert("C");
    for (auto& [nt, prods] : cmb.g_) {
        std::string new_nt = "C";
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (!cmb.st_.IsTerminal(symbol)) {
                    symbol = "C";
                    cmb.st_.non_terminals_.erase(symbol);
                }
            }
        }
        cmb_updated_grammar["C"] = prods;
    }
    cmb.g_ = std::move(cmb_updated_grammar);
    cmb.st_.non_terminals_.insert("C");

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
                    symbol = "C";
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

    return Grammar(combined_grammar);
}

Grammar GrammarFactory::Lv4() {
    // STEP 1 Build a random LV3 base grammar ------------------------------
    Grammar g = Lv3();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    // STEP 3 Change non terminals in cmb to C ---------------------------
    std::unordered_map<std::string, std::vector<production>>
        cmb_updated_grammar;
    cmb.st_.non_terminals_.insert("D");
    for (auto& [nt, prods] : cmb.g_) {
        std::string new_nt = "D";
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (!cmb.st_.IsTerminal(symbol)) {
                    symbol = "D";
                    cmb.st_.non_terminals_.erase(symbol);
                }
            }
        }
        cmb_updated_grammar["D"] = prods;
    }
    cmb.g_ = std::move(cmb_updated_grammar);
    cmb.st_.non_terminals_.insert("D");

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
                    symbol = "D";
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

    return Grammar(combined_grammar);
}

Grammar GrammarFactory::Lv5() {
    // STEP 1 Build a random LV3 base grammar ------------------------------
    Grammar g = Lv4();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    // STEP 3 Change non terminals in cmb to C ---------------------------
    std::unordered_map<std::string, std::vector<production>>
        cmb_updated_grammar;
    cmb.st_.non_terminals_.insert("E");
    for (auto& [nt, prods] : cmb.g_) {
        std::string new_nt = "E";
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (!cmb.st_.IsTerminal(symbol)) {
                    symbol = "E";
                    cmb.st_.non_terminals_.erase(symbol);
                }
            }
        }
        cmb_updated_grammar["E"] = prods;
    }
    cmb.g_ = std::move(cmb_updated_grammar);
    cmb.st_.non_terminals_.insert("E");

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
                    symbol = "E";
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

    return Grammar(combined_grammar);
}

Grammar GrammarFactory::Lv6() {
    // STEP 1 Build a random LV3 base grammar ------------------------------
    Grammar g = Lv5();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    // STEP 3 Change non terminals in cmb to C ---------------------------
    std::unordered_map<std::string, std::vector<production>>
        cmb_updated_grammar;
    cmb.st_.non_terminals_.insert("F");
    for (auto& [nt, prods] : cmb.g_) {
        std::string new_nt = "F";
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (!cmb.st_.IsTerminal(symbol)) {
                    symbol = "F";
                    cmb.st_.non_terminals_.erase(symbol);
                }
            }
        }
        cmb_updated_grammar["F"] = prods;
    }
    cmb.g_ = std::move(cmb_updated_grammar);
    cmb.st_.non_terminals_.insert("F");

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
                    symbol = "F";
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

    return Grammar(combined_grammar);
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

void GrammarFactory::RemoveLeftRecursion(Grammar& grammar) {
    if (!HasDirectLeftRecursion(grammar)) {
        return;
    }
    std::unordered_map<std::string, std::vector<production>> new_rules;
    for (const auto& [nt, productions] : grammar.g_) {
        std::vector<production> alpha;
        std::vector<production> beta;
        std::string             new_non_terminal = nt + "'";
        for (const auto& prod : productions) {
            if (!prod.empty() && prod[0] == nt) {
                alpha.push_back({prod.begin() + 1, prod.end()});
            } else {
                if (prod[0] == grammar.st_.EPSILON_) {
                    continue;
                }
                beta.push_back(prod);
            }
        }
        if (!alpha.empty()) {
            if (beta.empty()) {
                beta.push_back({});
            }
            for (auto& b : beta) {
                b.push_back(new_non_terminal);
            }
            for (auto& a : alpha) {
                a.push_back(new_non_terminal);
            }
            alpha.push_back({grammar.st_.EPSILON_});
            new_rules[nt]               = beta;
            new_rules[new_non_terminal] = alpha;
            grammar.st_.PutSymbol(new_non_terminal, false);
        } else {
            new_rules[nt] = productions;
        }
    }
    // EPSILON was introduced to the grammar, ensure it is in the symbol table
    grammar.st_.PutSymbol(grammar.st_.EPSILON_, true);
    grammar.g_ = std::move(new_rules);
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