#ifndef GRAPHREL_SRC_VARIABLE_ORDERING_HPP
#define GRAPHREL_SRC_VARIABLE_ORDERING_HPP

#include <vector>
#include <cstddef>

namespace teddy::graphrel {

// Forward declaration
struct graph;

/**
 * @brief Type of variable in decomposition ordering
 *
 * Based on Wu & Sun's algorithm, vertices are processed before their incident edges
 * to minimize boundary set size F_max.
 */
enum class variable_type {
    vertex,  // Vertex variable (vertex reliability)
    edge     // Edge variable (edge reliability)
};

/**
 * @brief Represents a single variable in the decomposition ordering
 */
struct variable {
    variable_type type;  // vertex or edge
    std::size_t id;      // Vertex ID or Edge ID

    variable(variable_type t, std::size_t i) : type(t), id(i) {}
};

/**
 * @brief Variable ordering strategy
 */
enum class ordering_strategy {
    bfs,  ///< Breadth-First Search (Wu & Sun default)
    dfs   ///< Depth-First Search
};

/**
 * @brief Generate variable ordering using BFS with vertex priority
 *
 * This implements the variable ordering strategy from Wu & Sun (2024):
 * - Use BFS starting from a terminal vertex
 * - Process vertices before their incident edges (vertex priority)
 * - Minimizes max boundary set size F_max
 *
 * Algorithm:
 * 1. Select seed vertex from terminals (first terminal)
 * 2. BFS traversal from seed
 * 3. For each vertex v:
 *    a. Add v to ordering (vertex type)
 *    b. For each edge e incident to v:
 *       - If both endpoints visited: add e to ordering (edge type)
 *       - If other endpoint not visited: enqueue it
 * 4. Add any remaining unvisited edges
 *
 * @param g Graph structure
 * @return Vector of variables in decomposition order
 */
std::vector<variable> generate_variable_ordering(const graph& g);

/**
 * @brief Generate variable ordering using specified strategy
 *
 * @param g Graph structure
 * @param strategy Ordering strategy (bfs or dfs)
 * @return Vector of variables in decomposition order
 */
std::vector<variable> generate_variable_ordering(const graph& g, ordering_strategy strategy);

/**
 * @brief Estimate maximum boundary set size for given ordering
 *
 * Simulates the ordering and tracks the size of the boundary set at each level.
 * The boundary set F at level l contains vertices incident to BOTH processed
 * and unprocessed edges.
 *
 * @param g Graph structure
 * @param ordering Variable ordering
 * @return Maximum boundary set size F_max
 */
std::size_t estimate_max_boundary_size(
    const graph& g,
    const std::vector<variable>& ordering
);

} // namespace teddy::graphrel

#endif // GRAPHREL_SRC_VARIABLE_ORDERING_HPP
