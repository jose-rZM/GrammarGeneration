#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/**
 * @brief Represents the type of a symbol in the grammar.
 *
 * This enumeration is used to distinguish between terminal and non-terminal
 * symbols in the grammar. It is typically used in symbol tables or during
 * grammar analysis.
 *
 * - `NO_TERMINAL`: Represents a non-terminal symbol, which can be expanded
 *   into sequences of terminals and/or other non-terminals using production
 * rules.
 * - `TERMINAL`: Represents a terminal symbol, which is a basic symbol in the
 *   grammar and cannot be further expanded.
 */
enum symbol_type { NO_TERMINAL, TERMINAL };

struct SymbolTable {
    /// @brief End-of-line symbol used in parsing, initialized as "$".
    std::string EOL_{"$"};

    /// @brief Epsilon symbol, representing empty transitions, initialized as
    /// "EPSILON".
    std::string EPSILON_{"EPSILON"};

    /// @brief Main symbol table, mapping identifiers to a pair of symbol type
    /// and its regex.
    std::unordered_map<std::string, symbol_type> st_{{EOL_, TERMINAL},
                                                     {EPSILON_, TERMINAL}};

    /**
     * @brief The set of terminal symbols in the grammar, including the
     * end-of-input marker (`EOL_`).
     *
     * This set contains all terminal symbols defined in the grammar, including
     * the special end-of-input marker (`EOL_`). It is used to distinguish
     * terminals from non-terminals during parsing and grammar analysis.
     */
    std::unordered_set<std::string> terminals_{EOL_};

    /**
     * @brief The set of terminal symbols in the grammar, excluding the
     * end-of-input marker (`EOL_`).
     *
     * This set contains all terminal symbols defined in the grammar, excluding
     * the special end-of-input marker (`EOL_`). It is used in contexts where
     * the end-of-input marker should not be treated as a regular terminal.
     */
    std::unordered_set<std::string> terminals_wtho_eol_{};

    /**
     * @brief The set of non-terminal symbols in the grammar.
     *
     * This set contains all non-terminal symbols defined in the grammar.
     * Non-terminals are symbols that can be expanded into sequences of
     * terminals and/or other non-terminals using production rules.
     */
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
     * @brief Checks if a symbol is a terminal excluding EOL.
     *
     * @param s Symbol identifier to check.
     * @return true if the symbol is terminal, otherwise false.
     */
    bool IsTerminalWthoEol(const std::string& s);
};
