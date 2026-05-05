#ifndef TEDDY_GRAPHREL_GRAPH_HPP
#define TEDDY_GRAPHREL_GRAPH_HPP

#include <algorithm>
#include <cstddef>
#include <vector>

/**
 * @file graph.hpp
 * @brief Graph representation for prob analysis
 *
 * Represents a graph with vertices, edges, reliabilities, and terminal
 * set. Uses adjacency list representation for efficient graph operations.
 */

namespace teddy::graphrel {

/**
 * @struct edge
 * @brief Represents a single edge in the graph
 */
struct edge {
  std::size_t id;     ///< Unique edge identifier (0-indexed)
  std::size_t from;   ///< Source vertex (0-indexed)
  std::size_t to;     ///< Target vertex (0-indexed)
  double prob; ///< Edge prob probability p_e[id]

  edge(std::size_t id_, std::size_t from_, std::size_t to_, double prob_)
      : id(id_), from(from_), to(to_), prob(prob_) {}
};

/**
 * @struct graph
 * @brief Complete graph representation for prob analysis
 *
 * Stores graph structure, component reliabilities, and terminal set K.
 * Uses adjacency list for efficient edge operations (delete, contract).
 */
struct graph {
  // Graph structure
  std::size_t num_vertices; ///< Number of vertices |V|
  std::size_t num_edges;    ///< Number of edges |E|

  // Vertex reliabilities (0-indexed)
  std::vector<double> vertex_probs; ///< p_v[i] for vertex i

  // Edge data
  std::vector<edge> edges; ///< All edges in the graph

  // Terminal vertices K ⊆ V (sorted vector)
  std::vector<std::size_t> terminals; ///< Sorted vector of terminal vertices (0-indexed)

  // Adjacency list: adj[v] = vector of edge IDs incident to vertex v
  std::vector<std::vector<std::size_t>> adj;

  /**
   * @brief Default constructor
   */
  graph() : num_vertices(0), num_edges(0) {}

  /**
   * @brief Constructor with size
   * @param n Number of vertices
   * @param m Number of edges
   */
  graph(std::size_t n, std::size_t m)
      : num_vertices(n), num_edges(m),
        vertex_probs(n, 1.0) // Default: perfect vertices
        ,
        adj(n) {
    edges.reserve(m);
  }

  /**
   * @brief Get neighbors of vertex v
   * @param v Vertex index (0-indexed)
   * @return Vector of neighbor vertex indices
   */
  std::vector<std::size_t> get_neighbors(std::size_t v) const;

  /**
   * @brief Get incident edges of vertex v
   * @param v Vertex index (0-indexed)
   * @return Vector of edge IDs incident to v
   */
  std::vector<std::size_t> get_incident_edges(std::size_t v) const;

  /**
   * @brief Check if vertex v is a terminal
   * @param v Vertex index (0-indexed)
   * @return true if v is in K
   */
  bool is_terminal(std::size_t v) const {
    return std::binary_search(terminals.begin(), terminals.end(), v);
  }

  /**
   * @brief Add edge to graph
   * @param from Source vertex (0-indexed)
   * @param to Target vertex (0-indexed)
   * @param prob Edge prob probability
   * @return Edge ID of newly added edge
   */
  std::size_t add_edge(std::size_t from, std::size_t to, double prob);

  /**
   * @brief Validate graph structure
   * @return true if graph is valid
   */
  bool validate() const;
};

} // namespace teddy::graphrel

#endif // TEDDY_GRAPHREL_GRAPH_HPP
