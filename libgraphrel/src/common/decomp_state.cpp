/**
 * @file decomp_state.cpp
 * @brief Subgraph state representation and terminal detection
 *
 * Implements subgraph state management and terminal connectivity detection
 * for the layer-by-layer algorithm. The subgraph state tracks:
 * - Remaining edges (not yet processed or removed)
 * - Union-Find structure (vertex connectivity from processed edges)
 * - Terminal set K (which terminals must remain connected)
 * - Validity flag (whether state represents a valid network configuration)
 */

#include "common/decomp_state.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <sstream>

namespace teddy::graphrel {

decomp_state::decomp_state(const graph &net, const std::vector<std::size_t> &K)
    : uf_(net.num_vertices) {
  /**
   * @brief Construct initial subgraph state from network
   *
   * Initializes the subgraph state for the original network:
   * - All edges are remaining (not yet processed)
   * - Union-Find is initialized with all vertices separate
   * - Terminal set K is set from network terminals (converted to sorted vector)
   * - State is valid (initial network is always valid)
   */
  // Initialize with all edges remaining (none processed yet)
  remaining_edges.resize(net.num_edges);
  for (std::size_t i = 0; i < net.num_edges; ++i) {
    remaining_edges[i] = i;
  }

  // Copy and sort the vector for consistent ordering
  current_K.assign(K.begin(), K.end());
  std::sort(current_K.begin(), current_K.end());
}

decomp_state decomp_state::clone() const {
  decomp_state copy;
  copy.remaining_edges = this->remaining_edges;
  copy.uf_ = this->uf_;
  copy.current_K = this->current_K;
  copy.is_valid = this->is_valid;
  return copy;
}

bool decomp_state::is_success_terminal() const {
  /**
   * @brief Check if all terminals are connected (success terminal)
   *
   * A success terminal state means all terminal vertices in K are in the
   * same connected component. This indicates the network is operational.
   *
   * Algorithm: Check if all terminals have the same Union-Find root.
   * If they do, they're all connected (through processed edges and remaining
   * edges that will be processed).
   *
   * @return true if all terminals are in the same component
   */
  // If fewer than 2 terminals, connectivity is trivially satisfied
  if (current_K.size() < 2) {
    return true;
  }

  // Check if all terminals share the same Union-Find root
  std::size_t first_root = uf_.find_root(*current_K.begin());
  for (std::size_t terminal : current_K) {
    if (uf_.find_root(terminal) != first_root) {
      return false; // Terminals in different components
    }
  }
  return true; // All terminals connected
}

bool decomp_state::is_failure_terminal(const graph &net,
                                        std::size_t ignored_edge_id) const {
  /**
   * @brief Check if terminals are disconnected (failure terminal)
   *
   * A failure terminal state means terminals are in different connected
   * components and cannot be connected by any remaining edges. This indicates
   * the network has failed.
   *
   * Algorithm:
   * 1. Create temporary Union-Find combining:
   *    - Current connectivity (from processed edges via union_find)
   *    - Future connectivity (from remaining unprocessed edges)
   * 2. Check if all terminals share the same root in this combined structure
   * 3. If not, terminals are disconnected -> failure terminal
   *
   * This is more robust than checking only processed edges because it considers
   * the full connectivity including edges that haven't been processed yet.
   *
   * @param net Original network (needed for edge information)
   * @param ignored_edge_id Optional edge ID to ignore when checking
   * connectivity (for coloop detection)
   * @return true if terminals are disconnected (failure terminal)
   */
  // If fewer than 2 terminals, cannot be disconnected
  if (current_K.size() < 2) {
    return false;
  }

  // OPTIMIZATION: Early exit if terminals are already connected via processed
  // edges If terminals are connected through processed edges, adding remaining
  // edges can only maintain or improve connectivity (never disconnect).
  // Therefore, if is_success_terminal() is true, is_failure_terminal() must be
  // false.
  if (is_success_terminal()) {
    return false;
  }

  // Build combined connectivity: processed edges (via union_find) + remaining
  // edges This gives us the complete connectivity picture, not just what's been
  // processed OPTIMIZATION: Copy existing union_find instead of rebuilding from
  // scratch This avoids O(n) unite operations and leverages existing
  // connectivity structure
  teddy::graphrel::union_find uf_check = uf_; // Copy existing state (fast vector copy)

  // Step 1: Merge vertices based on remaining edges (future connectivity)
  // This captures potential connectivity from edges not yet processed
  // Note: We don't need to rebuild processed edges - they're already in
  // union_find OPTIMIZATION: Skip ignored_edge_id if specified (for virtual
  // edge removal in coloop detection)
  for (std::size_t edge_id : remaining_edges) {
    if (edge_id == ignored_edge_id) {
      continue; // Skip this edge (virtual removal for coloop detection)
    }
    const auto &edge = net.edges[edge_id];
    uf_check.unite(edge.from, edge.to);
  }

  // Step 3: Check if all terminals are in the same component
  std::size_t first_root = uf_check.find(*current_K.begin());
  for (std::size_t terminal : current_K) {
    if (uf_check.find(terminal) != first_root) {
      return true; // Terminals disconnected -> failure terminal
    }
  }

  return false; // Terminals connected (or can be connected)
}

teddy::graphrel::union_find decomp_state::build_full_connectivity(const graph &net) const {
  teddy::graphrel::union_find uf_full = uf_; // Copy existing state
  for (std::size_t edge_id : remaining_edges) {
    const auto &edge = net.edges[edge_id];
    uf_full.unite(edge.from, edge.to);
  }
  return uf_full;
}

bool decomp_state::is_failure_terminal_with(const teddy::graphrel::union_find &full_uf) const {
  if (current_K.size() < 2) {
    return false;
  }
  std::size_t first_root = full_uf.find_root(*current_K.begin());
  for (std::size_t terminal : current_K) {
    if (full_uf.find_root(terminal) != first_root) {
      return true;
    }
  }
  return false;
}

bool decomp_state::operator==(const decomp_state &other) const {
  return remaining_edges == other.remaining_edges &&
         uf_ == other.uf_ && current_K == other.current_K &&
         is_valid == other.is_valid;
}

std::string decomp_state::to_string(const graph &net) const {
  std::ostringstream oss;
  oss << "decomp_state{\n";

  // Remaining edges
  oss << "  remaining_edges=[";
  for (std::size_t i = 0; i < remaining_edges.size(); ++i) {
    if (i > 0)
      oss << ",";
    oss << remaining_edges[i];
  }
  oss << "]\n";

  // Union-Find components for boundary vertices
  oss << "  union_find_components={";
  bool first = true;
  for (std::size_t v = 0; v < uf_.size(); ++v) {
    std::size_t root = uf_.find_root(v);
    if (root == v) { // Only print roots
      if (!first)
        oss << ",";
      first = false;
      oss << "v" << v << ":{";
      bool first_child = true;
      for (std::size_t u = 0; u < uf_.size(); ++u) {
        if (uf_.find_root(u) == v) {
          if (!first_child)
            oss << ",";
          first_child = false;
          oss << u;
        }
      }
      oss << "}";
    }
  }
  oss << "}\n";

  // Current K (terminals)
  oss << "  current_K=[";
  bool first_k = true;
  for (std::size_t k : current_K) {
    if (!first_k)
      oss << ",";
    first_k = false;
    oss << k;
  }
  oss << "]\n";

  // Union-Find root mapping for boundary vertices (critical for debugging)
  oss << "  boundary_vertex_roots={";
  std::set<std::size_t> boundary_vertices;
  for (std::size_t eid : remaining_edges) {
    if (eid < net.edges.size()) {
      boundary_vertices.insert(net.edges[eid].from);
      boundary_vertices.insert(net.edges[eid].to);
    }
  }
  bool first_bv = true;
  for (std::size_t v : boundary_vertices) {
    if (!first_bv)
      oss << ",";
    first_bv = false;
    std::size_t root = uf_.find_root(v);
    oss << "v" << v << "->" << root;
  }
  oss << "}\n";

  oss << "  is_valid=" << (is_valid ? "true" : "false") << "\n";
  oss << "}";
  return oss.str();
}

} // namespace teddy::graphrel
