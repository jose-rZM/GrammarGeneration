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
    };

    void Init();
    Grammar PickOne(int level);
    
    std::vector<FactoryItem> items;
    
};