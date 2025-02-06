#pragma once

#include "grammar.hpp"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct GrammarFactory {
    void Init();
    Grammar PickOne();
    
    std::vector<Grammar> items;
    
};