#ifndef GRAPHREL_SRC_GRAPH_OPERATIONS_HPP
#define GRAPHREL_SRC_GRAPH_OPERATIONS_HPP

#include "common/decomp_state.hpp"
#include "teddy/graphrel/graph.hpp"

/**
 * @file graph_operations.hpp
 * @brief Graph operations for network decomposition
 *
 * Implements edge deletion, contraction, vertex deletion, and special
 * case detection (coloop, loop) for Shannon decomposition.
 */

namespace teddy::graphrel {

/**
 * @brief Delete edge from decomp_state (edge fails)
 * @param sg Subgraph state
 * @param edge_id Edge ID to delete
 * @param net Original graph
 * @return New decomp_state state with edge removed
 */
decomp_state delete_edge(
    const decomp_state& sg,
    std::size_t edge_id,
    const graph& net
);

/**
 * @brief Contract edge in decomp_state (edge succeeds)
 * @param sg Subgraph state
 * @param edge_id Edge ID to contract
 * @param net Original graph
 * @return New decomp_state state with edge contracted
 */
decomp_state contract_edge(
    const decomp_state& sg,
    std::size_t edge_id,
    const graph& net
);

/**
 * @brief Delete vertex from decomp_state (vertex fails)
 * @param sg Subgraph state
 * @param vertex_id Vertex ID to delete
 * @param net Original graph
 * @return New decomp_state state with vertex and incident edges removed
 */
decomp_state delete_vertex(
    const decomp_state& sg,
    std::size_t vertex_id,
    const graph& net
);

/**
 * @brief Check if edge is a coloop (must succeed)
 * @param sg Subgraph state
 * @param edge_id Edge ID to check
 * @param net Original graph
 * @return true if edge is a coloop
 *
 * A coloop is an edge whose removal would disconnect terminals.
 */
bool is_coloop(
    const decomp_state& sg,
    std::size_t edge_id,
    const graph& net
);

/**
 * @brief Delete edge without internal is_failure_terminal check
 * @param sg Subgraph state
 * @param edge_id Edge ID to delete
 * @param net Original graph
 * @return New decomp_state state with edge removed (caller checks failure)
 *
 * Caller is responsible for checking is_failure_terminal on the result.
 * Use when the caller already has connectivity information cached.
 */
decomp_state delete_edge_no_check(
    const decomp_state& sg,
    std::size_t edge_id,
    const graph& net
);

/**
 * @brief Delete vertex without internal is_failure_terminal check
 * @param sg Subgraph state
 * @param vertex_id Vertex ID to delete
 * @param net Original graph
 * @return New decomp_state state with vertex and incident edges removed (caller checks failure)
 *
 * Caller is responsible for checking is_failure_terminal on the result.
 * Still marks as invalid if current_K becomes empty.
 */
decomp_state delete_vertex_no_check(
    const decomp_state& sg,
    std::size_t vertex_id,
    const graph& net
);

/**
 * @brief Delete edge in-place on an already-allocated state (no failure check)
 * @param sg State to modify in-place
 * @param edge_id Edge ID to delete
 *
 * Removes edge from remaining_edges. No clone, no failure check.
 */
void delete_edge_in_place(decomp_state& sg, std::size_t edge_id);

/**
 * @brief Contract edge in-place on an already-allocated state
 * @param sg State to modify in-place
 * @param edge_id Edge ID to contract
 * @param net Original graph (needed for edge endpoints and terminal update)
 */
void contract_edge_in_place(decomp_state& sg, std::size_t edge_id,
                            const graph& net);

/**
 * @brief Delete vertex in-place on an already-allocated state (no failure check)
 * @param sg State to modify in-place
 * @param vertex_id Vertex ID to delete
 * @param net Original graph (needed for incident edge list)
 *
 * Marks as invalid if current_K becomes empty.
 */
void delete_vertex_in_place(decomp_state& sg, std::size_t vertex_id,
                            const graph& net);

/**
 * @brief Check if edge is a loop (can be deleted)
 * @param sg Subgraph state
 * @param edge_id Edge ID to check
 * @param net Original graph
 * @return true if edge is a loop
 *
 * A loop is an edge whose endpoints are already in the same component
 * (from previous contractions).
 */
bool is_loop(
    const decomp_state& sg,
    std::size_t edge_id,
    const graph& net
);

} // namespace teddy::graphrel

#endif // GRAPHREL_SRC_GRAPH_OPERATIONS_HPP
