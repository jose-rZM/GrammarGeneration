#pragma once
#include "grammar.hpp"
#include <deque>
#include <queue>
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1Parser {
    using ll1_table = std::unordered_map<
        std::string, std::unordered_map<std::string, std::vector<production>>>;

  public:
    /**
     * @brief Constructs an LL1Parser with a grammar object and an input file.
     *
     * @param gr Grammar object to parse with
     */
    LL1Parser(Grammar gr);
    
    /**
     * @brief Creates the LL(1) parsing table for the grammar.
     *
     * This function constructs the LL(1) parsing table by iterating over each
     * production in the grammar and determining the appropriate cells for each
     * non-terminal and director symbol (prediction symbol) combination. If the
     * grammar is LL(1) compatible, each cell will contain at most one
     * production, indicating no conflicts. If conflicts are found, the function
     * will return `false`, signaling that the grammar is not LL(1).
     *
     * - For each production rule `A -> α`, the function calculates the director
     * symbols using the `director_symbols` function.
     * - It then fills the parsing table at the cell corresponding to the
     * non-terminal `A` and each director symbol in the set.
     * - If a cell already contains a production, this indicates a conflict,
     * meaning the grammar is not LL(1).
     *
     * @return `true` if the table is created successfully, indicating the
     * grammar is LL(1) compatible; `false` if any conflicts are detected,
     * showing that the grammar does not meet LL(1) requirements.
     */
    bool CreateLL1Table();

  private:
    /**
     * @brief Calculates the FIRST set for a given production rule in a grammar.
     *
     * The FIRST set of a production rule contains all terminal symbols that can
     * appear at the beginning of any string derived from that rule. If the rule
     * can derive the empty string (epsilon), epsilon is included in the FIRST
     * set.
     *
     * This function computes the FIRST set by examining each symbol in the
     * production rule:
     * - If a terminal symbol is encountered, it is added directly to the FIRST
     * set, as it is the starting symbol of some derivation.
     * - If a non-terminal symbol is encountered, its FIRST set is recursively
     * computed and added to the result, excluding epsilon unless it is followed
     * by another symbol that could also lead to epsilon.
     * - If the entire rule could derive epsilon (i.e., each symbol in the rule
     * can derive epsilon), then epsilon is added to the FIRST set.
     *
     * @param rule A span of strings representing the production rule for which
     * to compute the FIRST set. Each string in the span is a symbol (either
     * terminal or non-terminal).
     * @param result A reference to an unordered set of strings where the
     * computed FIRST set will be stored. The set will contain all terminal
     * symbols that can start derivations of the rule, and possibly epsilon if
     * the rule can derive an empty string.
     */
    void First(std::span<const std::string>     rule,
               std::unordered_set<std::string>& result);

    /**
     * @brief Computes the FIRST sets for all non-terminal symbols in the
     * grammar.
     *
     * This function calculates the FIRST set for each non-terminal symbol in
     * the grammar by iteratively applying a least fixed-point algorithm. This
     * approach ensures that the FIRST sets are fully populated by repeatedly
     * expanding and updating the sets until no further changes occur (i.e., a
     * fixed-point is reached).
     */
    void ComputeFirstSets();

    /**
     * @brief Computes the FOLLOW set for a given non-terminal symbol in the
     * grammar.
     *
     * The FOLLOW set for a non-terminal symbol includes all symbols that can
     * appear immediately to the right of that symbol in any derivation, as well
     * as any end-of-input markers if the symbol can appear at the end of
     * derivations. FOLLOW sets are used in LL(1) parsing table construction to
     * determine possible continuations after a non-terminal.
     *
     * This function initiates the calculation and uses `follow_util` as a
     * recursive helper to handle dependencies among non-terminals and avoid
     * redundant computations.
     *
     * @param arg Non-terminal symbol for which to compute the FOLLOW set.
     * @return An unordered set of strings containing symbols that form the
     * FOLLOW set for `arg`.
     */
    std::unordered_set<std::string> Follow(const std::string& arg);

    
    /**
     * @brief Recursive utility function to compute the FOLLOW set for a
     * non-terminal.
     *
     * This function assists in building the FOLLOW set by handling recursive
     * dependencies among non-terminals, ensuring that cycles are properly
     * managed to avoid infinite recursion. The helper function performs
     * depth-first traversal through the production rules to collect symbols
     * that should belong to the FOLLOW set of the target non-terminal.
     *
     * - If a non-terminal appears in a production, `follow_util` gathers
     * symbols immediately following it in that production.
     * - If no symbols follow the target non-terminal or if the remaining
     * symbols can derive epsilon, it incorporates symbols from the FOLLOW set
     * of the non-terminal on the left-hand side of the production rule.
     *
     * @param arg The non-terminal symbol whose FOLLOW set is being computed.
     * @param visited An unordered set of strings used to track symbols already
     * visited in the current recursion path, preventing infinite loops.
     * @param next_symbols An unordered set to accumulate symbols forming the
     * FOLLOW set of the target non-terminal as they are discovered.
     */
    void FollowUtil(const std::string&               arg,
                    std::unordered_set<std::string>& visited,
                    std::unordered_set<std::string>& next_symbols);

    
 /**
     * @brief Computes the prediction symbols for a given
     * production rule.
     *
     * The prediction symbols for a rule,
     * determine the set of input symbols that can trigger this rule in the
     * parsing table. This function calculates the prediction symbols based on
     * the FIRST set of the consequent and, if epsilon (the empty symbol) is in
     * the FIRST set, also includes the FOLLOW set of the antecedent.
     *
     * - If the FIRST set of the consequent does not contain epsilon, the
     * prediction symbols are simply the FIRST symbols of the consequent.
     * - If the FIRST set of the consequent contains epsilon, the prediction
     * symbols are computed as (FIRST(consequent) - {epsilon}) ∪
     * FOLLOW(antecedent).
     *
     * @param antecedent The left-hand side non-terminal symbol of the rule.
     * @param consequent A vector of symbols on the right-hand side of the rule
     * (production body).
     * @return An unordered set of strings containing the prediction symbols for
     * the specified rule.
     */
    std::unordered_set<std::string>
    PredictionSymbols(const std::string&              antecedent,
                      const std::vector<std::string>& consequent);


    /// @brief The LL(1) parsing table, mapping non-terminals and terminals to
    /// productions.
    ll1_table ll1_t_;

    /// @brief Grammar object associated with this parser.
    Grammar gr_;

    /// @brief FIRST sets for each non-terminal in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>> first_sets;
};
