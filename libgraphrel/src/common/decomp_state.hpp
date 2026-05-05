#ifndef GRAPHREL_SRC_DECOMP_STATE_HPP
#define GRAPHREL_SRC_DECOMP_STATE_HPP

#include "teddy/graphrel/graph.hpp"
#include "common/union_find.hpp"
#include <climits> // For SIZE_MAX
#include <string>
#include <vector>

/**
 * @file decomp_state.hpp
 * @brief Decomposition state representation for network decomposition
 *
 * Represents a subgraph during the layer-by-layer decomposition process.
 * Tracks remaining edges, vertex connectivity, and terminal set.
 */

namespace teddy::graphrel {

/**
 * @brief Represents the state of a subgraph during decomposition
 */
struct decomp_state {
  /**
   * @brief Remaining edge IDs in this subgraph
   *
   * INVARIANT: Always sorted in ascending order. This enables O(log n)
   * binary_search for ghost edge detection in reliability_calculator.cpp.
   */
  std::vector<std::size_t> remaining_edges;
  union_find uf_;
  std::vector<std::size_t>
      current_K; // Sorted vector (replaces std::set for better cache locality)
  bool is_valid = true;

  // For pool allocator free list (similar to DecisionDiagrams node_pool)
  decomp_state *next_ = nullptr;

  // Default constructor for map creation
  decomp_state() : uf_(0) {}

  /**
   * @brief Constructor for initial state
   * @param net The network
   * @param K The set of terminal vertices
   */
  decomp_state(
      const graph &net,
      const std::vector<std::size_t>
          &K);

  /**
   * @brief Create a deep copy of the subgraph state
   * @return A new subgraph instance
   */
  decomp_state clone() const;

  /**
   * @brief Check if all terminals are connected (success terminal)
   * @return true if all terminals in same component
   */
  bool is_success_terminal() const;

  /**
   * @brief Check if terminals are disconnected (failure terminal)
   * @param net graph reference (needed to check connectivity through
   * remaining edges)
   * @param ignored_edge_id Optional edge ID to ignore when checking
   * connectivity (for coloop detection)
   * @return true if terminals are truly disconnected (no path through remaining
   * edges)
   */
  bool is_failure_terminal(const graph &net,
                           std::size_t ignored_edge_id = SIZE_MAX) const;

  /**
   * @brief Build a union_find combining current connectivity + all remaining edges
   *
   * This is the expensive part of is_failure_terminal. By computing it once
   * and reusing, we avoid redundant O(V+E) work in isColoop and deleteEdge.
   *
   * @param net graph reference
   * @return union_find with full (current + remaining) connectivity
   */
  teddy::graphrel::union_find build_full_connectivity(const graph &net) const;

  /**
   * @brief Check failure terminal using a pre-built full connectivity UF
   *
   * O(|K|) instead of O(V+E) -- just checks if terminals share a root.
   *
   * @param full_uf Pre-built full connectivity union_find
   * @return true if terminals are disconnected
   */
  bool is_failure_terminal_with(const teddy::graphrel::union_find &full_uf) const;

  // Equality for testing and map keys
  bool operator==(const decomp_state &other) const;

  /**
   * @brief Convert to human-readable string for debugging
   * @param net graph reference (needed for edge information)
   * @return String representation
   */
  std::string to_string(const graph &net) const;
};

} // namespace teddy::graphrel

#endif // GRAPHREL_SRC_DECOMP_STATE_HPP
