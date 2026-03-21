#ifndef TEDDY_GRAPHREL_DIAGRAM_HPP
#define TEDDY_GRAPHREL_DIAGRAM_HPP

#include "teddy/graphrel/graph.hpp"
#include <libteddy/reliability.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @file diagram.hpp
 * @brief BDD-based network prob (Wu & Sun 2024, BS-BDD)
 *
 * Builds a Binary Decision Diagram representing the network structure function
 * using boundary-set partition isomorphism detection. The resulting reliability_diagram
 * owns a bss_manager and gives access to the full TeDDy analysis API:
 * calculate_probability, dpld, importance measures, MCVs, etc.
 */

namespace teddy::graphrel {

/**
 * @brief Move-only bundle: BDD manager + diagram + per-variable probabilities.
 *
 * Returned by build_diagram(). Owns the bss_manager so the BDD stays alive.
 */
struct reliability_diagram {
    teddy::bss_manager            manager;
    teddy::bss_manager::diagram_t diagram;
    std::vector<double>           probs;   ///< prob per TeDDy variable index

    /**
     * @brief Return the TeDDy variable index for a graph edge.
     * @return -1 if the edge has no variable (perfect edge, prob=1).
     */
    teddy::int32 edge_var(std::size_t edge_id) const;

    /**
     * @brief Return the TeDDy variable index for a graph vertex.
     * @return -1 if vertex has no variable (perfect vertex, prob=1).
     */
    teddy::int32 vertex_var(std::size_t vertex_id) const;

    /**
     * @brief Create a var_change descriptor for dpld analysis on an edge.
     */
    teddy::var_change edge_change(std::size_t edge_id,
                                  teddy::int32 from,
                                  teddy::int32 to) const;

    // Move-constructible; move assignment deleted (bss_manager has no move assign)
    reliability_diagram(reliability_diagram&&) = default;
    reliability_diagram& operator=(reliability_diagram&&) = delete;
    reliability_diagram(const reliability_diagram&) = delete;
    reliability_diagram& operator=(const reliability_diagram&) = delete;

    // Internal constructor (used by build_diagram)
    reliability_diagram(teddy::bss_manager mgr,
              teddy::bss_manager::diagram_t diag,
              std::vector<double> edge_probs,
              std::vector<teddy::int32> edge_to_var,
              std::vector<teddy::int32> vertex_to_var);

private:
    std::vector<teddy::int32> edge_to_var_;   ///< graph edge_id → TeDDy var index
    std::vector<teddy::int32> vertex_to_var_; ///< graph vertex_id → TeDDy var index
};

/**
 * @brief Build a BDD for network prob.
 *
 * Uses the BS-BDD algorithm (Wu & Sun 2024) with boundary-set partition
 * isomorphism detection. Returns a reliability_diagram owning the manager.
 *
 * @param g         Graph with edge/vertex reliabilities and terminal set.
 * @param pool_size Initial TeDDy node pool size (default 100 000).
 * @return          Populated reliability_diagram ready for analysis.
 */
reliability_diagram build_diagram(const graph& g, teddy::int64 pool_size = 100'000);

} // namespace teddy::graphrel

#endif // TEDDY_GRAPHREL_DIAGRAM_HPP
