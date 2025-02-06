#include <map>
#include <string>
#include <span>
#include <unordered_set>

#include "grammar.hpp"
#include "lr0_item.hpp"
#include "state.hpp"

class SLR1Parser
{
public:
    enum class Action
    {
        Shift,
        Reduce,
        Accept,
        Empty
    };
    struct s_action
    {
        const Lr0Item *item;
        Action action;
    };
    using action_table =
        std::map<unsigned int, std::map<std::string, SLR1Parser::s_action>>;
    using transition_table =
        std::map<unsigned int, std::map<std::string, unsigned int>>;
    SLR1Parser(Grammar gr);
    std::unordered_set<Lr0Item> allItems() const;
    void DebugStates() const;
    void DebugActions() const;
    void DebugTable() const;
    void Closure(std::unordered_set<Lr0Item> &items);
    void ClosureUtil(std::unordered_set<Lr0Item> &items, unsigned int size,
                     std::unordered_set<std::string> &visited);
    bool SolveLRConflicts(const state &st);

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
    void First(std::span<const std::string> rule,
               std::unordered_set<std::string> &result);
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
    std::unordered_set<std::string> Follow(const std::string &arg);
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
    void FollowUtil(const std::string &arg,
                    std::unordered_set<std::string> &visited,
                    std::unordered_set<std::string> &next_symbols);

    void MakeInitialStat();
    bool MakeParser();

    Grammar gr_;
    std::unordered_map<std::string, std::unordered_set<std::string>> first_sets;
    action_table actions_;
    transition_table transitions_;
    std::unordered_set<state> states_;
};
