#include "grammar.hpp"
#include "grammar_error.hpp"
#include "symbol_table.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <utility>
#include <vector>

Grammar::Grammar(const std::unordered_map<std::string, std::vector<production>> &grammar) {
    for (const auto &[nt, prods] : grammar) {
        st_.PutSymbol(nt, false);
        for (const auto& prod : prods) {
            for (const std::string &symbol : prod) {
                if (symbol == "EPSILON") {
                    continue;
                } else if (std::islower(symbol[0])) {
                    st_.PutSymbol(symbol, true);
                }
            }
        }
    }
    axiom_ = "S";
    g_ = std::move(grammar);
    if (g_.find("S") == g_.end()) {
        // S -> firstNonTerminal $
        g_["S"] = {{*st_.non_terminals_.begin(), st_.EOL_}};
    }
}


/*void Grammar::AddRule(const std::string& antecedent,
                      const std::string& consequent) {
    std::vector<std::string> splitted_consequent{Split(consequent)};
    if (HasLeftRecursion(antecedent, splitted_consequent)) {
        throw GrammarError("Grammar has left recursion, it can't be LL1.");
    }
    g_[antecedent].push_back(splitted_consequent);
}*/

void Grammar::SetAxiom(const std::string& axiom) {
    axiom_ = axiom;
}

bool Grammar::HasEmptyProduction(const std::string& antecedent) {
    auto rules{g_.at(antecedent)};
    return std::find_if(rules.cbegin(), rules.cend(), [&](const auto& rule) {
               return rule[0] == st_.EPSILON_;
           }) != rules.cend();
}

std::vector<std::pair<const std::string, production>>
Grammar::FilterRulesByConsequent(const std::string& arg) {
    std::vector<std::pair<const std::string, production>> rules;
    for (const std::pair<const std::string, std::vector<production>>& rule :
         g_) {
        for (const production& prod : rule.second) {
            if (std::find(prod.cbegin(), prod.cend(), arg) != prod.cend()) {
                rules.emplace_back(rule.first, prod);
            }
        }
    }
    return rules;
}

void Grammar::Debug() {
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
bool Grammar::HasLeftRecursion(const std::string&              antecedent,
                               const std::vector<std::string>& consequent) {
    return consequent.at(0) == antecedent;
}
