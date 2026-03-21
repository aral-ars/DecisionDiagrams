#ifndef GRAPHREL_SRC_BOUNDARY_HPP
#define GRAPHREL_SRC_BOUNDARY_HPP

#include "common/partition.hpp"
#include "common/bell_cache.hpp"   // declares bell_cache (renamed to bell_cache in C.3)
#include "common/decomp_state.hpp"
#include "teddy/graphrel/variable_ordering.hpp"
#include "teddy/graphrel/graph.hpp"

#include <cstddef>
#include <limits>
#include <unordered_map>
#include <vector>

/**
 * @file boundary.hpp
 * @brief Shared boundary partition computation for LBL and BDD algorithms
 *
 * Eliminates code duplication between lbl_calculator and bdd_builder:
 * both algorithms require identical boundary vertex identification and
 * partition canonicalization logic.
 */

namespace teddy::graphrel {

/**
 * @class boundary_computer
 * @brief Precomputed boundary metadata + per-call partition computation.
 *
 * Constructed once from (graph, ordering). Precomputes per-level sets of
 * vertices incident to processed / unprocessed edges, then identifies the
 * boundary vertices (incident to both) for any (decomp_state, level) pair.
 *
 * Thread-safety: identify_vertices() uses thread_local bitmaps — safe for
 * concurrent use from *different* threads, not re-entrant within one thread.
 */
class boundary_computer {
public:
    boundary_computer(const graph& g, const std::vector<variable>& ordering);

    boundary_partition compute(const decomp_state& sg, std::size_t level,
                               const bell_cache& cache) const;

private:
    struct level_boundary_data {
        std::vector<std::size_t> vertices_with_processed_edges;
        std::vector<std::size_t> vertices_with_unprocessed_edges;
    };

    void precompute();
    std::vector<std::size_t> identify_vertices(const decomp_state& sg,
                                               std::size_t level) const;

    const graph& net_;
    const std::vector<variable>& ordering_;
    std::unordered_map<std::size_t, std::size_t> edge_level_map_;
    std::vector<std::size_t> vertex_introduction_level_;
    std::vector<level_boundary_data> level_boundary_cache_;
};

} // namespace teddy::graphrel

#endif // GRAPHREL_SRC_BOUNDARY_HPP
