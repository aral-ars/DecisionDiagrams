/**
 * @file graph.cpp
 * @brief Graph structure implementation
 *
 * Implements graph operations including edge management, neighbor
 * queries, and graph validation. The graph uses an adjacency list
 * representation for efficient edge operations during decomposition.
 */

#include "teddy/graphrel/graph.hpp"
#include <algorithm>
#include <stdexcept>

namespace teddy::graphrel {

std::vector<std::size_t> graph::get_neighbors(std::size_t v) const {
  if (v >= num_vertices) {
    throw std::out_of_range("Vertex index out of range");
  }

  std::vector<std::size_t> neighbors;
  neighbors.reserve(adj[v].size());

  for (std::size_t edge_id : adj[v]) {
    const edge &e = edges[edge_id];
    std::size_t neighbor = (e.from == v) ? e.to : e.from;
    neighbors.push_back(neighbor);
  }

  return neighbors;
}

std::vector<std::size_t> graph::get_incident_edges(std::size_t v) const {
  if (v >= num_vertices) {
    throw std::out_of_range("Vertex index out of range");
  }
  return adj[v];
}

std::size_t graph::add_edge(std::size_t from, std::size_t to,
                            double prob) {
  /**
   * @brief Add edge to graph
   *
   * Adds an edge between two vertices with the specified prob.
   * The edge is assigned a unique ID and added to the adjacency lists
   * of both endpoints.
   *
   * @param from Source vertex (0-indexed)
   * @param to Target vertex (0-indexed)
   * @param prob Edge prob probability (must be in [0, 1])
   * @return Edge ID of newly added edge
   * @throws std::out_of_range if vertex indices are invalid
   * @throws std::invalid_argument if prob is outside [0, 1]
   */
  if (from >= num_vertices || to >= num_vertices) {
    throw std::out_of_range("Vertex index out of range");
  }
  if (prob < 0.0 || prob > 1.0) {
    throw std::invalid_argument("Reliability must be in [0, 1]");
  }

  // Assign edge ID (sequential, 0-indexed)
  std::size_t edge_id = edges.size();
  edges.emplace_back(edge_id, from, to, prob);

  // Add edge to adjacency lists of both endpoints
  adj[from].push_back(edge_id);
  if (from != to) { // Avoid duplicate entry for self-loops
    adj[to].push_back(edge_id);
  }

  return edge_id;
}

bool graph::validate() const {
  /**
   * @brief Validate graph structure integrity
   *
   * Performs comprehensive validation checks to ensure the graph is
   * well-formed and ready for prob calculation:
   * - Vertex/edge count consistency
   * - Reliability values in valid range [0, 1]
   * - Edge vertex indices within bounds
   * - Terminal vertices are valid
   * - Adjacency lists are consistent with edge list
   *
   * @return true if graph is valid, false otherwise
   */
  // Check 1: Vertex count consistency
  // All vertex-related arrays must have the same size
  if (vertex_probs.size() != num_vertices) {
    return false;
  }
  if (adj.size() != num_vertices) {
    return false;
  }

  // Check 2: Edge count consistency
  if (edges.size() != num_edges) {
    return false;
  }

  // Check 3: Vertex reliabilities are in valid range [0, 1]
  for (double p : vertex_probs) {
    if (p < 0.0 || p > 1.0) {
      return false;
    }
  }

  // Check 4: Edge data integrity
  for (const edge &e : edges) {
    // Edge prob must be in [0, 1]
    if (e.prob < 0.0 || e.prob > 1.0) {
      return false;
    }
    // Edge endpoints must be valid vertex indices
    if (e.from >= num_vertices || e.to >= num_vertices) {
      return false;
    }
    // Edge ID must be within valid range
    if (e.id >= num_edges) {
      return false;
    }
  }

  // Check 5: Terminal vertices are valid
  // All terminals must be valid vertex indices
  for (std::size_t t : terminals) {
    if (t >= num_vertices) {
      return false;
    }
  }

  // Check 6: Adjacency list consistency
  // Every edge ID in adjacency lists must:
  // - Exist in edges array
  // - Have the vertex as one of its endpoints
  for (std::size_t v = 0; v < num_vertices; ++v) {
    for (std::size_t edge_id : adj[v]) {
      if (edge_id >= edges.size()) {
        return false; // Edge ID out of range
      }
      const edge &e = edges[edge_id];
      // Vertex must be an endpoint of the edge
      if (e.from != v && e.to != v) {
        return false; // Adjacency list inconsistency
      }
    }
  }

  return true; // All checks passed
}

} // namespace teddy::graphrel
