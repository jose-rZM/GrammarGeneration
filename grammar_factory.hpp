#pragma once

#include "grammar.hpp"
#include "symbol_table.hpp"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @struct GrammarFactory
 * @brief Responsible for creating and managing grammar items and performing
 * checks on grammars.
 */
struct GrammarFactory {

    /**
     * @struct FactoryItem
     * @brief Represents an individual grammar item with its associated symbol
     * table.
     */
    struct FactoryItem {
        /**
         * @brief Stores the grammar rules where each key is a non-terminal
         * symbol and each value is a vector of production rules.
         */
        std::unordered_map<std::string, std::vector<production>> g_;

        /**
         * @brief Symbol table associated with this grammar item.
         */
        symbol_table st_;

        /**
         * @brief Constructor that initializes a FactoryItem with the provided
         * grammar.
         * @param grammar The grammar to initialize the FactoryItem with.
         */
        FactoryItem(const std::unordered_map<std::string,
                                             std::vector<production>>& grammar);

        /**
         * @brief Checks if a grammar has an empty production for a given
         * non-terminal.
         * @param antecedent The non-terminal symbol to check for empty
         * productions.
         * @return true if there is an empty production, false otherwise.
         */
        bool HasEmptyProduction(const std::string& antecedent);

        /**
         * @brief Debugging function to print the grammar item.
         */
        void Debug();
    };

    /**
     * @brief Initializes the GrammarFactory and populates the items vector with
     * initial grammar items.
     */
    void Init();

    /**
     * @brief Picks a random grammar based on the specified difficulty level (1,
     * 2, or 3).
     * @param level The difficulty level (1, 2, or 3).
     * @return A randomly picked grammar.
     */
    Grammar PickOne(int level);

    Grammar GenLL1Grammar(int level);
    Grammar GenSLR1Grammar(int level);
    void    SanityChecks(Grammar& gr);

  private:
    /**
     * @brief Generates a Level 1 grammar.
     * @return A Level 1 grammar.
     */
    Grammar Lv1();

    /**
     * @brief Generates a Level 2 grammar by combining Level 1 items.
     * @return A Level 2 grammar.
     */
    Grammar Lv2();

    /**
     * @brief Generates a Level 3 grammar by combining a Level 2 item and a
     * Level 1 item.
     * @return A Level 3 grammar.
     */
    Grammar Lv3();

    FactoryItem CreateLv2Item();

    // -------- SANITY CHECKS --------

    /**
     * @brief Checks if a grammar contains unreachable symbols (non-terminals
     * that cannot be derived from the start symbol).
     * @param grammar The grammar to check.
     * @return true if there are unreachable symbols, false otherwise.
     */
    bool HasUnreachableSymbols(Grammar& grammar);

    /**
     * @brief Checks if a grammar is infinite, meaning there are non-terminal
     * symbols that can never derive a terminal string. This happens when a
     * production leads to an infinite recursion or an endless derivation
     * without reaching terminal symbols. For example, a production like: S -> A
     * A -> a A | B
     * B -> c B
     * could lead to an infinite derivation of non-terminals.
     * @param grammar The grammar to check.
     * @return true if the grammar has infinite derivations, false otherwise.
     */
    bool IsInfinite(Grammar& grammar);

    /**
     * @brief Checks if a grammar contains direct left recursion (a non-terminal
     * can produce itself on the left side of a production in one step).
     * @param grammar The grammar to check.
     * @return true if there is direct left recursion, false otherwise.
     */
    bool HasDirectLeftRecursion(Grammar& grammar);

    // -------- TRANSFORMATIONS --------
    void RemoveLeftRecursion(Grammar& grammar);
    /**
     * @brief A vector of FactoryItem objects representing different level 1
     * grammar items created by the Init method.
     */
    std::vector<FactoryItem> items;

    /**
     * @brief A vector of terminal symbols (alphabet) used in the grammar.
     */
    std::vector<std::string> terminal_alphabet_{"a", "b", "c", "d", "e"};

    /**
     * @brief A vector of non-terminal symbols (alphabet) used in the grammar.
     */
    std::vector<std::string> non_terminal_alphabet_{"A", "B", "C", "D", "E"};
};
