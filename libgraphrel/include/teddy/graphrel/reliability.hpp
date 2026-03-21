#ifndef TEDDY_GRAPHREL_RELIABILITY_HPP
#define TEDDY_GRAPHREL_RELIABILITY_HPP

#include "teddy/graphrel/graph.hpp"
#include "teddy/graphrel/variable_ordering.hpp"

/**
 * @file reliability.hpp
 * @brief Layer-by-layer network reliability (Wu & Sun 2024)
 *
 * Computes reliability directly as a probability (no BDD allocated).
 * Use build_diagram() instead when DPLD / importance / MCS analysis is needed.
 */

namespace teddy::graphrel {

/**
 * @brief Calculate network reliability using the layer-by-layer algorithm.
 *
 * @param g Graph with edge/vertex reliabilities and terminal set.
 * @return Probability that all terminals are connected.
 */
double calculate_reliability(const graph& g);

/**
 * @brief Calculate network reliability with a custom variable ordering.
 *
 * @param g        Graph with edge/vertex reliabilities and terminal set.
 * @param ordering Variable ordering (e.g. from generate_variable_ordering).
 * @return Probability that all terminals are connected.
 */
double calculate_reliability(const graph& g,
                             const std::vector<variable>& ordering);

} // namespace teddy::graphrel

#endif // TEDDY_GRAPHREL_RELIABILITY_HPP
