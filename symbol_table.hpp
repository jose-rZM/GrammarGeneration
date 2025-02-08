#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
    /// @brief End-of-line symbol used in parsing, initialized as "$".
    std::string EOL_{"$"};

    /// @brief Epsilon symbol, representing empty transitions, initialized as
    /// "EPSILON".
    std::string EPSILON_{"EPSILON"};

    /// @brief Main symbol table, mapping identifiers to a pair of symbol type
    /// and its regex.
    std::unordered_map<std::string, symbol_type>
        st_{{EOL_, TERMINAL}, {EPSILON_, TERMINAL}};

    std::unordered_set<std::string> terminals_ = {EOL_, EPSILON_};
    std::unordered_set<std::string> non_terminals_;

    /**
     * @brief Adds a non-terminal symbol to the symbol table.
     *
     * @param identifier Name of the  symbol.
     * @param isTerminal True if the identifier is a terminal symbol
     */
    void PutSymbol(const std::string& identifier, bool isTerminal);

    /**
     * @brief Checks if a symbol exists in the symbol table.
     *
     * @param s Symbol identifier to search.
     * @return true if the symbol is present, otherwise false.
     */
    bool In(const std::string& s);

    /**
     * @brief Checks if a symbol is a terminal.
     *
     * @param s Symbol identifier to check.
     * @return true if the symbol is terminal, otherwise false.
     */
    bool IsTerminal(const std::string& s);


    /**
     * @brief Prints all symbols and their properties in the symbol table.
     *
     * Outputs the symbol table for debugging purposes.
     */
    void Debug();
};
