#include "grammar.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

Grammar::Grammar(
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
    axiom_ = "S";
    g_     = grammar;
    g_["S"] = {{"A", st_.EOL_}};
    st_.PutSymbol(axiom_, false);
}

void Grammar::TransformToAugmentedGrammar() {
    std::string new_axiom = axiom_ + "'";
    g_[new_axiom] = {{axiom_}};
    st_.PutSymbol(new_axiom, false);

    axiom_ = new_axiom;
}


void Grammar::SetAxiom(const std::string& axiom) {
    axiom_ = axiom;
}

bool Grammar::HasEmptyProduction(const std::string& antecedent) {
    auto rules{g_.at(antecedent)};
    return std::ranges::find_if(rules, [&](const auto& rule) {
               return rule[0] == st_.EPSILON_;
           }) != rules.cend();
}

std::vector<std::pair<const std::string, production>>
Grammar::FilterRulesByConsequent(const std::string& arg) {
    std::vector<std::pair<const std::string, production>> rules;
    for (const auto& [lhs, productions] : g_) {
        for (const production& prod : productions) {
            if (std::ranges::find(prod, arg) != prod.cend()) {
                rules.emplace_back(lhs, prod);
            }
        }
    }
    return rules;
}

void Grammar::Debug() {
    std::cout << "Grammar:\n";

    std::cout << axiom_ << " -> ";
    const auto& axiom_productions = g_.at(axiom_);
    for (size_t i = 0; i < axiom_productions.size(); ++i) {
        for (const std::string& symbol : axiom_productions[i]) {
            std::cout << symbol << " ";
        }
        if (i < axiom_productions.size() - 1) {
            std::cout << "| ";
        }
    }
    std::cout << "\n";

    std::vector<std::string> non_terminals;
    for (const auto& [lhs, _] : g_) {
        if (lhs != axiom_) {
            non_terminals.push_back(lhs);
        }
    }

    std::ranges::sort(non_terminals);

    for (const std::string& nt : non_terminals) {
        std::cout << nt << " -> ";
        const auto& productions = g_.at(nt);
        for (size_t i = 0; i < productions.size(); ++i) {
            for (const std::string& symbol : productions[i]) {
                std::cout << symbol << " ";
            }
            if (i < productions.size() - 1) {
                std::cout << "| ";
            }
        }
        std::cout << "\n";
    }
}

bool Grammar::HasLeftRecursion(const std::string&              antecedent,
                               const std::vector<std::string>& consequent) {
    return consequent.at(0) == antecedent;
}

void Grammar::AddProduction(const std::string&              antecedent,
                            const std::vector<std::string>& consequent) {
    g_[antecedent].push_back(consequent);
}
