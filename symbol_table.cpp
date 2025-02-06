#include "symbol_table.hpp"
#include <cstdio>
#include <unordered_map>
#include <vector>


void symbol_table::PutSymbol(const std::string& identifier, bool isTerminal) {
    if (isTerminal) {
        st_[identifier]          = {TERMINAL};
        terminals_.insert(identifier);

    } else {
        st_.insert({identifier, NO_TERMINAL});
        non_terminals_.insert(identifier);
    }
}

bool symbol_table::In(const std::string& s) {
    return st_.find(s) != st_.cend();
}

bool symbol_table::IsTerminal(const std::string& s) {
    return terminals_.find(s) != terminals_.end();
}