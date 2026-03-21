#ifndef GRAPHREL_SRC_PRUNING_HPP
#define GRAPHREL_SRC_PRUNING_HPP

#include "teddy/graphrel/graph.hpp"
#include "common/decomp_state.hpp"

/**
 * @file pruning.hpp
 * @brief Dead component pruning for network decomposition
 *
 * Removes edges incident to "dead" components -- connected components that
 * contain no terminal vertices and have degree <= 1. Such components cannot
 * contribute to terminal connectivity, so pruning them enables more states
 * to merge (states differing only in dead-component structure become
 * isomorphic after pruning).
 */

namespace teddy::graphrel {

/**
 * @brief Prune edges incident to dead (non-terminal, degree<=1) components
 *
 * Applied iteratively: removing one edge may expose new dead components.
 * Modifies state->remaining_edges in place.
 *
 * @param state Subgraph state to prune
 * @param net Graph reference (for edge endpoint lookup)
 */
void prune_unreachable_edges(decomp_state &state, const graph &net);

} // namespace teddy::graphrel

#endif // GRAPHREL_SRC_PRUNING_HPP
