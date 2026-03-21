/**
 * @file boundary.cpp
 * @brief boundary_computer: shared boundary partition computation
 *
 * Used by both lbl_calculator (LBL algorithm) and bdd_builder (BS-BDD).
 * Uses thread-local bitmaps for O(1) per-vertex lookup with no heap
 * allocation in the steady state.
 */

#include "common/boundary.hpp"

#include <set>
#include <vector>

namespace teddy::graphrel {

boundary_computer::boundary_computer(const graph& g,
                                     const std::vector<variable>& ordering)
    : net_(g)
    , ordering_(ordering)
{
    edge_level_map_.reserve(g.num_edges);
    vertex_introduction_level_.assign(g.num_vertices,
                                      std::numeric_limits<std::size_t>::max());

    for (std::size_t level = 0; level < ordering_.size(); ++level) {
        const variable& var = ordering_[level];
        if (var.type == variable_type::edge) {
            edge_level_map_[var.id] = level;
        } else if (var.type == variable_type::vertex) {
            if (vertex_introduction_level_[var.id] ==
                    std::numeric_limits<std::size_t>::max())
                vertex_introduction_level_[var.id] = level;
        }
    }

    precompute();
}

boundary_partition boundary_computer::compute(const decomp_state& sg,
                                              std::size_t level,
                                              const bell_cache& cache) const {
    std::vector<std::size_t> boundary = identify_vertices(sg, level);
    return canonicalize_partition(boundary, sg.union_find, sg.current_K, cache);
}

void boundary_computer::precompute() {
    level_boundary_cache_.resize(ordering_.size() + 1);

    for (std::size_t level = 0; level <= ordering_.size(); ++level) {
        level_boundary_data& data = level_boundary_cache_[level];
        std::set<std::size_t> processed_cands;
        std::set<std::size_t> unprocessed_cands;

        for (std::size_t v = 0; v < net_.num_vertices; ++v) {
            if (vertex_introduction_level_[v] < level)
                processed_cands.insert(v);
        }

        for (std::size_t edge_id = 0; edge_id < net_.num_edges; ++edge_id) {
            auto it = edge_level_map_.find(edge_id);
            if (it == edge_level_map_.end())
                continue;

            std::size_t edge_level = it->second;
            const auto& e = net_.edges[edge_id];

            if (edge_level < level) {
                processed_cands.insert(e.from);
                if (e.from != e.to)
                    processed_cands.insert(e.to);
            } else {
                unprocessed_cands.insert(e.from);
                if (e.from != e.to)
                    unprocessed_cands.insert(e.to);
            }
        }

        data.vertices_with_processed_edges.assign(processed_cands.begin(),
                                                   processed_cands.end());
        data.vertices_with_unprocessed_edges.assign(unprocessed_cands.begin(),
                                                     unprocessed_cands.end());
    }
}

std::vector<std::size_t>
boundary_computer::identify_vertices(const decomp_state& sg,
                                     std::size_t level) const {
    if (level >= level_boundary_cache_.size())
        level = level_boundary_cache_.size() - 1;

    const level_boundary_data& data = level_boundary_cache_[level];
    const auto& with_processed = data.vertices_with_processed_edges;

    // Thread-local bitmap: O(1) lookup, no heap allocation in steady state.
    static thread_local std::vector<bool> has_unprocessed;
    if (has_unprocessed.size() < net_.num_vertices)
        has_unprocessed.resize(net_.num_vertices, false);

    // Track which entries we set for sparse clear.
    static thread_local std::vector<std::size_t> touched;
    touched.clear();

    for (std::size_t edge_id : sg.remaining_edges) {
        if (edge_id >= net_.num_edges)
            continue;
        auto it = edge_level_map_.find(edge_id);
        if (it == edge_level_map_.end())
            continue;

        if (it->second >= level) {
            const auto& e = net_.edges[edge_id];
            if (!has_unprocessed[e.from]) {
                has_unprocessed[e.from] = true;
                touched.push_back(e.from);
            }
            if (e.from != e.to && !has_unprocessed[e.to]) {
                has_unprocessed[e.to] = true;
                touched.push_back(e.to);
            }
        }
    }

    std::vector<std::size_t> result;
    for (std::size_t vid : with_processed) {
        if (vid < has_unprocessed.size() && has_unprocessed[vid])
            result.push_back(vid);
    }

    // Sparse clear
    for (std::size_t vid : touched)
        has_unprocessed[vid] = false;

    return result;
}

} // namespace teddy::graphrel
