#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "symbol_table.hpp"

using production = std::vector<std::string>;

struct Grammar
{

    Grammar() = default;
    explicit Grammar(
        const std::unordered_map<std::string, std::vector<production>> &grammar);

    /**
     * @brief Adds a rule to the grammar.
     *
     * @param antecedent The left-hand side (LHS) symbol of the rule.
     * @param consequent The right-hand side (RHS) of the rule as a string.
     *
     * Adds a rule to the grammar by specifying the antecedent symbol and the
     * consequent production. This function processes and adds each rule for
     * parsing.
     */
    void AddRule(const std::string &antecedent, const std::string &consequent);

    /**
     * @brief Sets the axiom (entry point) of the grammar.
     *
     * @param axiom The entry point or start symbol of the grammar.
     *
     * Defines the starting point for the grammar, which is used in parsing
     * algorithms and must be a non-terminal symbol present in the grammar.
     */
    void SetAxiom(const std::string &axiom);

    /**
     * @brief Checks if a given antecedent has an empty production.
     *
     * @param antecedent The left-hand side (LHS) symbol to check.
     * @return true if there exists an empty production for the antecedent,
     *         otherwise false.
     *
     * An empty production is represented as `<antecedent> -> ;`, indicating
     * that the antecedent can produce an empty string.
     */
    bool HasEmptyProduction(const std::string &antecedent);

    /**
     * @brief Filters grammar rules that contain a specific token in their
     * consequent.
     *
     * @param arg The token to search for within the consequents of the rules.
     * @return std::vector of pairs where each pair contains an antecedent and
     * its respective production that includes the specified token.
     *
     * Searches for rules in which the specified token is part of the consequent
     * and returns those rules.
     */
    std::vector<std::pair<const std::string, production>>
    FilterRulesByConsequent(const std::string &arg);

    /**
     * @brief Prints the current grammar structure to standard output.
     *
     * This function provides a debug view of the grammar by printing out all
     * rules, the axiom, and other relevant details.
     */
    void Debug();

    /**
     * @brief Splits a production string into individual tokens.
     *
     * @param s The production string to split.
     * @return std::vector containing tokens extracted from the production
     * string.
     *
     * The function decomposes a production string into individual symbols based
     * on the symbol table, allowing terminals and non-terminals to be
     * identified.
     */
    std::vector<std::string> Split(const std::string &s);

    /**
     * @brief Checks if a rule exhibits left recursion.
     *
     * @param antecedent The left-hand side (LHS) symbol of the rule.
     * @param consequent The right-hand side (RHS) vector of tokens of the rule.
     * @return true if the rule has left recursion (e.g., A -> A + A), otherwise
     * false.
     *
     * Left recursion is identified when the antecedent of a rule appears as the
     * first symbol in its consequent, which may cause issues in top-down
     * parsing algorithms.
     */
    bool HasLeftRecursion(const std::string &antecedent,
                          const std::vector<std::string> &consequent);

    /**
     * @brief Stores the grammar rules with each antecedent mapped to a list of
     * productions.
     */
    std::unordered_map<std::string, std::vector<production>> g_;

    /**
     * @brief The axiom or entry point of the grammar.
     */
    std::string axiom_;

    symbol_table st_;
};
