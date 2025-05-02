#pragma once

#include <string>
#include <vector>

/**
 * @brief Represents an LR(0) item in the grammar.
 *
 * An LR(0) item consists of a production rule with a dot (•) indicating the
 * current position in the rule. It is used during the construction of the
 * LR(0) state machine for parsing.
 *
 * @var antecedent_ The non-terminal symbol on the left-hand side of the
 * production.
 * @var consequent_ The sequence of symbols on the right-hand side of the
 * production.
 * @var epsilon_ The epsilon symbol (empty string) used in the grammar.
 * @var eol_ The end-of-input marker used in the grammar.
 * @var dot_ The position of the dot in the production (default is 0).
 */
struct Lr0Item {
    std::string antecedent_; ///< The non-terminal symbol on the left-hand side.
    std::vector<std::string>
        consequent_;       ///< The sequence of symbols on the right-hand side.
    std::string  epsilon_; ///< The epsilon symbol (empty string).
    std::string  eol_;     ///< The end-of-input marker.
    unsigned int dot_ = 0; ///< The position of the dot in the production.

    /**
     * @brief Constructs an LR(0) item with the dot at the beginning.
     *
     * @param antecedent The non-terminal symbol on the left-hand side.
     * @param consequent The sequence of symbols on the right-hand side.
     * @param epsilon The epsilon symbol (empty string).
     * @param eol The end-of-input marker.
     */
    Lr0Item(std::string antecedent, std::vector<std::string> consequent,
            std::string epsilon, std::string eol);

    /**
     * @brief Constructs an LR(0) item with the dot at a specific position.
     *
     * @param antecedent The non-terminal symbol on the left-hand side.
     * @param consequent The sequence of symbols on the right-hand side.
     * @param dot The position of the dot in the production.
     * @param epsilon The epsilon symbol (empty string).
     * @param eol The end-of-input marker.
     */
    Lr0Item(std::string antecedent, std::vector<std::string> consequent,
            unsigned int dot, std::string epsilon, std::string eol);

    /**
     * @brief Returns the symbol immediately after the dot.
     *
     * @return The symbol after the dot, or an empty string if the dot is at the
     * end.
     */
    std::string NextToDot() const;

    /**
     * @brief Prints the LR(0) item to the standard output.
     */
    void PrintItem() const;

    /**
     * @brief Converts the LR(0) item to a string representation.
     *
     * @return A string representation of the LR(0) item.
     */
    std::string ToString() const;

    /**
     * @brief Advances the dot position by one.
     */
    void AdvanceDot();

    /**
     * @brief Checks if the LR(0) item is complete (i.e., the dot is at the
     * end).
     *
     * @return `true` if the dot is at the end of the production, `false`
     * otherwise.
     */
    bool IsComplete() const;

    /**
     * @brief Compares two LR(0) items for equality.
     *
     * Two LR(0) items are considered equal if they have the same antecedent,
     * consequent, and dot position.
     *
     * @param other The LR(0) item to compare with.
     * @return `true` if the items are equal, `false` otherwise.
     */
    bool operator==(const Lr0Item& other) const;
};

/**
 * @brief Specialization of `std::hash` for the `Lr0Item` struct.
 *
 * This specialization allows `Lr0Item` objects to be used as keys in unordered
 * containers (e.g., `std::unordered_set` or `std::unordered_map`). The hash
 * value is computed based on the antecedent, consequent, and dot position.
 */
namespace std {
template <> struct hash<Lr0Item> {
    /**
     * @brief Computes the hash value for an `Lr0Item` object.
     *
     * The hash value is computed by combining the hash values of the
     * antecedent, consequent, and dot position.
     *
     * @param item The LR(0) item for which to compute the hash value.
     * @return The computed hash value.
     */
    size_t operator()(const Lr0Item& item) const;
};
} // namespace std
