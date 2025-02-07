#include "grammar_factory.hpp"
#include <random>
#include <algorithm>
#include <iostream>

void GrammarFactory::Init()
{
    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "b", "A"}, {"a"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "b", "A"}, {"a", "b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A", "b"}, {"EPSILON"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"A", "a"}, {"EPSILON"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A"}, {"EPSILON"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A", "c"}, {"b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A", "a"}, {"b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"A", "a"}, {"b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"b", "A"}, {"a"}}}});
}

Grammar GrammarFactory::PickOne(int level)
{
    switch (level)
    {
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

Grammar GrammarFactory::Lv1()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

Grammar GrammarFactory::Lv2()
{
    // STEP 1 Choose a random base grammar ----------------------------------
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem base = items.at(dist(gen));
    // -----------------------------------------------------

    // STEP 2 Choose a random cmb grammar such that base != cmb ------------------------------
    FactoryItem cmb = items.at(dist(gen));
    while (base.g_ == cmb.g_) {
        cmb = items.at(dist(gen));
    }
    // -----------------------------------------------------

// DEBUG -------------------------------------------------------------
    std::cout << "----------------------\n";
    std::cout << "BASE GRAMMAR\n";
    base.Debug();
    std::cout << "----------------------\n";
    std::cout << "CMB GRAMMAR\n";
    cmb.Debug();
    std::cout << "----------------------\n";


// END DEBUG -------------------------------------------------------------

    // STEP 3 Change non terminals in cmb to B --------------------------------------------
    std::unordered_map<std::string, std::vector<production>> cmb_updated_grammar;
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
    std::unordered_set<std::string> cmb_terminals = cmb.st_.terminals_;
    std::unordered_set<std::string> terminal_alphabet_set(terminal_alphabet_.begin(), terminal_alphabet_.end());

    for (const std::string& terminal : cmb_terminals) {
        terminal_alphabet_set.erase(terminal);
    }

    std::uniform_int_distribution<size_t> terminal_dist(0, terminal_alphabet_set.size() - 1);
    std::vector<std::string> remaining_terminals(terminal_alphabet_set.begin(), terminal_alphabet_set.end());
    std::string new_terminal = remaining_terminals[terminal_dist(gen)];

    std::uniform_int_distribution<size_t> base_terminal_dist(0, base.st_.terminals_.size() - 1);
    std::vector<std::string> base_terminals(base.st_.terminals_.begin(), base.st_.terminals_.end());
    std::string terminal_to_replace = base_terminals.at(base_terminal_dist(gen));

    for (auto& [nt, prods] : base.g_) {
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (symbol == terminal_to_replace) {
                    symbol = new_terminal;
                }
            }
        }
    }
    base.st_.terminals_.erase(terminal_to_replace);
    base.st_.terminals_.insert(new_terminal);
    // -----------------------------------------------------
    
    // STEP 5 Change one random terminal -> terminal B
    terminal_to_replace = *std::next(base.st_.terminals_.begin(), base_terminal_dist(gen));
    for (auto& [nt, prods] : base.g_) {
        for (auto& prod : prods) {
            for (std::string& symbol : prod) {
                if (symbol == terminal_to_replace) {
                    symbol = "B";
                }
            }
        }
    }

    std::unordered_map<std::string, std::vector<production>> combined_grammar = base.g_;
    for (auto& [nt, prods] : cmb.g_) {
        combined_grammar[nt].insert(combined_grammar[nt].end(), prods.begin(), prods.end());
    }

    return Grammar(combined_grammar);
    
}

Grammar GrammarFactory::Lv3()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

GrammarFactory::FactoryItem::FactoryItem(const std::unordered_map<std::string, std::vector<production>> &grammar)
{
    for (const auto &[nt, prods] : grammar)
    {
        st_.PutSymbol(nt, false);
        for (const auto &prod : prods)
        {
            for (const std::string &symbol : prod)
            {
                if (symbol == "EPSILON")
                {
                    continue;
                }
                else if (std::islower(symbol[0]))
                {
                    st_.PutSymbol(symbol, true);
                }
            }
        }
    }
    g_ = (grammar);
}

bool GrammarFactory::FactoryItem::HasEmptyProduction(const std::string &antecedent)
{
    auto &rules = g_.at(antecedent);
    return std::find_if(rules.cbegin(), rules.cend(), [&](const auto &rule)
                        { return rule[0] == "EPSILON"; }) != rules.cend();
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