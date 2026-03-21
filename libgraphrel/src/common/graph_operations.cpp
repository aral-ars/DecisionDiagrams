/**
 * @file graph_operations.cpp
 * @brief Graph operations for network decomposition (Shannon decomposition)
 */

#include "common/graph_operations.hpp"
#include <algorithm>

namespace teddy::graphrel {

decomp_state delete_edge(const decomp_state &sg, std::size_t edge_id,
                     const graph &net) {
    decomp_state result = sg.clone();

    auto it = std::lower_bound(result.remaining_edges.begin(),
                               result.remaining_edges.end(), edge_id);
    if (it != result.remaining_edges.end() && *it == edge_id) {
        result.remaining_edges.erase(it);
    }

    if (result.is_failure_terminal(net)) {
        result.is_valid = false;
    }

    return result;
}

decomp_state contract_edge(const decomp_state &sg, std::size_t edge_id,
                       const graph &net) {
    decomp_state result = sg.clone();

    const edge &e = net.edges[edge_id];
    std::size_t u = e.from;
    std::size_t v = e.to;

    result.union_find.unite(u, v);

    auto it = std::lower_bound(result.remaining_edges.begin(),
                               result.remaining_edges.end(), edge_id);
    if (it != result.remaining_edges.end() && *it == edge_id) {
        result.remaining_edges.erase(it);
    }

    auto u_it = std::find(result.current_K.begin(), result.current_K.end(), u);
    auto v_it = std::find(result.current_K.begin(), result.current_K.end(), v);
    bool u_in_K = (u_it != result.current_K.end());
    bool v_in_K = (v_it != result.current_K.end());

    if (u_in_K && v_in_K) {
        std::size_t rep = result.union_find.find(u);
        result.current_K.erase(u_it);
        v_it = std::find(result.current_K.begin(), result.current_K.end(), v);
        if (v_it != result.current_K.end()) {
            result.current_K.erase(v_it);
        }
        auto rep_it = std::lower_bound(result.current_K.begin(),
                                       result.current_K.end(), rep);
        if (rep_it == result.current_K.end() || *rep_it != rep) {
            result.current_K.insert(rep_it, rep);
        }
    } else if (v_in_K) {
        std::size_t rep = result.union_find.find(u);
        result.current_K.erase(v_it);
        auto rep_it = std::lower_bound(result.current_K.begin(),
                                       result.current_K.end(), rep);
        if (rep_it == result.current_K.end() || *rep_it != rep) {
            result.current_K.insert(rep_it, rep);
        }
    } else if (u_in_K) {
        std::size_t rep = result.union_find.find(u);
        if (rep != u) {
            result.current_K.erase(u_it);
            auto rep_it = std::lower_bound(result.current_K.begin(),
                                           result.current_K.end(), rep);
            if (rep_it == result.current_K.end() || *rep_it != rep) {
                result.current_K.insert(rep_it, rep);
            }
        }
    }

    return result;
}

decomp_state delete_vertex(const decomp_state &sg, std::size_t vertex_id,
                       const graph &net) {
    decomp_state result = sg.clone();

    auto it = std::lower_bound(result.current_K.begin(),
                               result.current_K.end(), vertex_id);
    if (it != result.current_K.end() && *it == vertex_id) {
        result.current_K.erase(it);
    }

    const auto &incident = net.adj[vertex_id];
    for (std::size_t eid : incident) {
        auto eit = std::lower_bound(result.remaining_edges.begin(),
                                    result.remaining_edges.end(), eid);
        if (eit != result.remaining_edges.end() && *eit == eid) {
            result.remaining_edges.erase(eit);
        }
    }

    if (result.current_K.empty() || result.is_failure_terminal(net)) {
        result.is_valid = false;
    }

    return result;
}

bool is_coloop(const decomp_state &sg, std::size_t edge_id, const graph &net) {
    const edge &e = net.edges[edge_id];
    std::size_t u = e.from;
    std::size_t v = e.to;

    if (sg.union_find.connected_const(u, v)) {
        return false;
    }

    if (std::binary_search(sg.remaining_edges.begin(),
                           sg.remaining_edges.end(), edge_id)) {
        if (sg.is_failure_terminal(net, edge_id)) {
            return true;
        }
    }

    return false;
}

decomp_state delete_edge_no_check(const decomp_state &sg, std::size_t edge_id,
                              const graph & /*net*/) {
    decomp_state result = sg.clone();

    auto it = std::lower_bound(result.remaining_edges.begin(),
                               result.remaining_edges.end(), edge_id);
    if (it != result.remaining_edges.end() && *it == edge_id) {
        result.remaining_edges.erase(it);
    }

    return result;
}

decomp_state delete_vertex_no_check(const decomp_state &sg, std::size_t vertex_id,
                                const graph &net) {
    decomp_state result = sg.clone();

    auto it = std::lower_bound(result.current_K.begin(),
                               result.current_K.end(), vertex_id);
    if (it != result.current_K.end() && *it == vertex_id) {
        result.current_K.erase(it);
    }

    const auto &incident = net.adj[vertex_id];
    for (std::size_t eid : incident) {
        auto eit = std::lower_bound(result.remaining_edges.begin(),
                                    result.remaining_edges.end(), eid);
        if (eit != result.remaining_edges.end() && *eit == eid) {
            result.remaining_edges.erase(eit);
        }
    }

    if (result.current_K.empty()) {
        result.is_valid = false;
    }

    return result;
}

void delete_edge_in_place(decomp_state &sg, std::size_t edge_id) {
    auto it = std::lower_bound(sg.remaining_edges.begin(),
                               sg.remaining_edges.end(), edge_id);
    if (it != sg.remaining_edges.end() && *it == edge_id) {
        sg.remaining_edges.erase(it);
    }
}

void contract_edge_in_place(decomp_state &sg, std::size_t edge_id,
                            const graph &net) {
    const edge &e = net.edges[edge_id];
    std::size_t u = e.from;
    std::size_t v = e.to;

    sg.union_find.unite(u, v);

    auto it = std::lower_bound(sg.remaining_edges.begin(),
                               sg.remaining_edges.end(), edge_id);
    if (it != sg.remaining_edges.end() && *it == edge_id) {
        sg.remaining_edges.erase(it);
    }

    auto u_it = std::find(sg.current_K.begin(), sg.current_K.end(), u);
    auto v_it = std::find(sg.current_K.begin(), sg.current_K.end(), v);
    bool u_in_K = (u_it != sg.current_K.end());
    bool v_in_K = (v_it != sg.current_K.end());

    if (u_in_K && v_in_K) {
        std::size_t rep = sg.union_find.find(u);
        sg.current_K.erase(u_it);
        v_it = std::find(sg.current_K.begin(), sg.current_K.end(), v);
        if (v_it != sg.current_K.end()) {
            sg.current_K.erase(v_it);
        }
        auto rep_it =
            std::lower_bound(sg.current_K.begin(), sg.current_K.end(), rep);
        if (rep_it == sg.current_K.end() || *rep_it != rep) {
            sg.current_K.insert(rep_it, rep);
        }
    } else if (v_in_K) {
        std::size_t rep = sg.union_find.find(u);
        sg.current_K.erase(v_it);
        auto rep_it =
            std::lower_bound(sg.current_K.begin(), sg.current_K.end(), rep);
        if (rep_it == sg.current_K.end() || *rep_it != rep) {
            sg.current_K.insert(rep_it, rep);
        }
    } else if (u_in_K) {
        std::size_t rep = sg.union_find.find(u);
        if (rep != u) {
            sg.current_K.erase(u_it);
            auto rep_it = std::lower_bound(sg.current_K.begin(),
                                           sg.current_K.end(), rep);
            if (rep_it == sg.current_K.end() || *rep_it != rep) {
                sg.current_K.insert(rep_it, rep);
            }
        }
    }
}

void delete_vertex_in_place(decomp_state &sg, std::size_t vertex_id,
                            const graph &net) {
    auto it = std::lower_bound(sg.current_K.begin(), sg.current_K.end(),
                               vertex_id);
    if (it != sg.current_K.end() && *it == vertex_id) {
        sg.current_K.erase(it);
    }

    const auto &incident = net.adj[vertex_id];
    for (std::size_t eid : incident) {
        auto eit = std::lower_bound(sg.remaining_edges.begin(),
                                    sg.remaining_edges.end(), eid);
        if (eit != sg.remaining_edges.end() && *eit == eid) {
            sg.remaining_edges.erase(eit);
        }
    }

    if (sg.current_K.empty()) {
        sg.is_valid = false;
    }
}

bool is_loop(const decomp_state &sg, std::size_t edge_id, const graph &net) {
    const edge &e = net.edges[edge_id];
    return sg.union_find.connected_const(e.from, e.to);
}

} // namespace teddy::graphrel
