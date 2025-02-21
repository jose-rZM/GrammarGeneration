#pragma once
#include "lr0_item.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <unordered_set>

/**
 * @brief Represents a state in the LR(0) state machine.
 *
 * A state consists of a set of LR(0) items and a unique identifier. It is used
 * to represent a specific configuration of the parser during the construction
 * of the parsing tables.
 */
struct state {
    ///@brief The set of LR(0) items in this state.
    std::unordered_set<Lr0Item> items_;
    /// @brief Unique identifier for this state.
    unsigned int id_;

    /**
     * @brief Compares two states for equality.
     *
     * Two states are considered equal if they contain the same set of LR(0)
     * items.
     *
     * @param other The state to compare with.
     * @return `true` if the states are equal, `false` otherwise.
     */
    bool operator==(const state& other) const { return other.items_ == items_; }
};

/**
 * @brief Specialization of `std::hash` for the `state` struct.
 *
 * This specialization allows `state` objects to be used as keys in unordered
 * containers (e.g., `std::unordered_set` or `std::unordered_map`). The hash
 * value is computed by combining the hash values of all LR(0) items in the
 * state.
 */
namespace std {
/**
 * @brief Computes the hash value for a `state` object.
 *
 * The hash value is computed by accumulating the hash values of all LR(0)
 * items in the state using the XOR operation.
 *
 * @param st The state for which to compute the hash value.
 * @return The computed hash value.
 */
template <> struct hash<state> {
    size_t operator()(const state& st) const {
        size_t seed =
            std::accumulate(st.items_.begin(), st.items_.end(), 0,
                            [](size_t acc, const Lr0Item& item) {
                                return acc ^ (std::hash<Lr0Item>()(item));
                            });
        return seed;
    }
};
} // namespace std
