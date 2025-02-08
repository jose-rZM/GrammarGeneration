#pragma once

#include "grammar.hpp"
#include "symbol_table.hpp"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct GrammarFactory {
    struct FactoryItem {
        std::unordered_map<std::string, std::vector<production>> g_;
        symbol_table st_;
        FactoryItem(const std::unordered_map<std::string, std::vector<production>>& grammar);
        bool HasEmptyProduction(const std::string& antecedent);
        void Debug();
    };

    void Init();
    Grammar PickOne(int level);
    Grammar Lv1();
    Grammar Lv2();
    Grammar Lv3();

    // SANITY CHECKS --------
    bool IsInfinite(Grammar& grammar);
    bool HasDirectLeftRecursion(Grammar& grammar);
    
    std::vector<FactoryItem> items;
    std::vector<std::string> terminal_alphabet_ {"a", "b", "c", "d", "e"};
    std::vector<std::string> non_terminal_alphabet_ {"A", "B", "C", "D", "E"};
    
};