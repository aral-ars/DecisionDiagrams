#include "teddy/graphrel/graph_io.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace teddy::graphrel {

// ---------------------------------------------------------------------------
// ParseError
// ---------------------------------------------------------------------------

ParseError::ParseError(std::size_t line, const std::string &message)
    : std::runtime_error("line " + std::to_string(line) + ": " + message),
      line_(line) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

/// Trim leading whitespace (in-place).
std::string_view trim_left(std::string_view sv) {
  auto pos = sv.find_first_not_of(" \t\r");
  return pos == std::string_view::npos ? std::string_view{} : sv.substr(pos);
}

/// Return true if the line is blank or a comment.
bool is_skip_line(std::string_view sv) {
  sv = trim_left(sv);
  return sv.empty() || sv.front() == 'c';
}

} // namespace

// ---------------------------------------------------------------------------
// read_graph (stream)
// ---------------------------------------------------------------------------

graph read_graph(std::istream &input) {
  // State accumulated during parsing
  bool saw_problem = false;
  bool saw_terminals = false;
  std::size_t num_vertices = 0;
  std::size_t num_edges = 0;
  double default_edge_rel = 1.0;
  double default_vertex_rel = 1.0;

  struct RawEdge {
    std::size_t from, to;
    double prob;
  };
  std::vector<RawEdge> raw_edges;

  std::vector<std::size_t> terminals;
  std::vector<std::pair<std::size_t, double>> vertex_overrides;

  std::string line;
  std::size_t line_number = 0;

  while (std::getline(input, line)) {
    ++line_number;

    if (is_skip_line(line))
      continue;

    std::istringstream iss(line);
    char prefix{};
    iss >> prefix;

    switch (prefix) {

    case 'p': {
      if (saw_problem)
        throw ParseError(line_number, "duplicate 'p' line");

      std::string kind;
      iss >> kind;
      if (kind != "reliability")
        throw ParseError(line_number,
                         "expected 'p reliability ...', got 'p " + kind + "'");

      if (!(iss >> num_vertices >> num_edges))
        throw ParseError(line_number,
                         "expected 'p reliability <V> <E> [defaults]'");

      // Optional defaults
      if (iss >> default_edge_rel) {
        if (default_edge_rel < 0.0 || default_edge_rel > 1.0)
          throw ParseError(line_number,
                           "default edge prob out of range [0, 1]");
        if (iss >> default_vertex_rel) {
          if (default_vertex_rel < 0.0 || default_vertex_rel > 1.0)
            throw ParseError(line_number,
                             "default vertex prob out of range [0, 1]");
        }
      }

      raw_edges.reserve(num_edges);
      saw_problem = true;
      break;
    }

    case 't': {
      if (!saw_problem)
        throw ParseError(line_number, "'t' line before 'p' line");
      if (saw_terminals)
        throw ParseError(line_number, "duplicate 't' line");

      std::size_t v;
      while (iss >> v) {
        if (v >= num_vertices)
          throw ParseError(line_number,
                           "terminal vertex " + std::to_string(v) +
                               " out of range [0, " +
                               std::to_string(num_vertices) + ")");
        terminals.push_back(v);
      }

      if (terminals.empty())
        throw ParseError(line_number, "terminal set must not be empty");

      std::sort(terminals.begin(), terminals.end());
      terminals.erase(std::unique(terminals.begin(), terminals.end()),
                      terminals.end());

      saw_terminals = true;
      break;
    }

    case 'e': {
      if (!saw_problem)
        throw ParseError(line_number, "'e' line before 'p' line");

      std::size_t from, to;
      if (!(iss >> from >> to))
        throw ParseError(line_number, "expected 'e <from> <to> [prob]'");

      if (from >= num_vertices || to >= num_vertices)
        throw ParseError(line_number,
                         "edge endpoint out of range [0, " +
                             std::to_string(num_vertices) + ")");

      double rel = default_edge_rel;
      if (iss >> rel) {
        if (rel < 0.0 || rel > 1.0)
          throw ParseError(line_number,
                           "edge prob out of range [0, 1]");
      }

      raw_edges.push_back({from, to, rel});
      break;
    }

    case 'v': {
      if (!saw_problem)
        throw ParseError(line_number, "'v' line before 'p' line");

      std::size_t vertex;
      double rel;
      if (!(iss >> vertex >> rel))
        throw ParseError(line_number,
                         "expected 'v <vertex> <prob>'");

      if (vertex >= num_vertices)
        throw ParseError(line_number,
                         "vertex " + std::to_string(vertex) +
                             " out of range [0, " +
                             std::to_string(num_vertices) + ")");

      if (rel < 0.0 || rel > 1.0)
        throw ParseError(line_number,
                         "vertex prob out of range [0, 1]");

      vertex_overrides.emplace_back(vertex, rel);
      break;
    }

    default:
      throw ParseError(line_number,
                        std::string("unknown line type '") + prefix + "'");
    }
  }

  // Post-parse validation
  if (!saw_problem)
    throw ParseError(line_number, "missing required 'p reliability' line");

  if (!saw_terminals)
    throw ParseError(line_number, "missing required 't' terminal line");

  if (raw_edges.size() != num_edges)
    throw ParseError(line_number,
                     "expected " + std::to_string(num_edges) + " edges, got " +
                         std::to_string(raw_edges.size()));

  // Build graph
  graph g(num_vertices, num_edges);

  std::fill(g.vertex_probs.begin(), g.vertex_probs.end(),
            default_vertex_rel);

  for (auto &[vertex, rel] : vertex_overrides)
    g.vertex_probs[vertex] = rel;

  for (auto &[from, to, rel] : raw_edges)
    g.add_edge(from, to, rel);

  g.terminals = std::move(terminals);

  return g;
}

// ---------------------------------------------------------------------------
// read_graph (file path)
// ---------------------------------------------------------------------------

graph read_graph(std::string_view path) {
  std::string path_str(path);
  std::ifstream file(path_str);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path_str);
  }
  return read_graph(file);
}

// ---------------------------------------------------------------------------
// write_graph (stream)
// ---------------------------------------------------------------------------

void write_graph(std::ostream &output, const graph &g,
                 const std::string &comment) {
  // Comments
  if (!comment.empty()) {
    std::istringstream iss(comment);
    std::string line;
    while (std::getline(iss, line))
      output << "c " << line << "\n";
  }

  // Problem line — always write defaults as 1.0
  output << "p reliability " << g.num_vertices << " " << g.edges.size()
         << "\n";

  // Terminals
  output << "t";
  for (auto v : g.terminals)
    output << " " << v;
  output << "\n";

  // Edges — always write explicit prob for round-trip fidelity
  output << std::setprecision(15);
  for (const auto &e : g.edges)
    output << "e " << e.from << " " << e.to << " " << e.prob << "\n";

  // Vertex reliabilities — only write non-1.0 vertices
  for (std::size_t i = 0; i < g.num_vertices; ++i) {
    if (g.vertex_probs[i] != 1.0)
      output << "v " << i << " " << g.vertex_probs[i] << "\n";
  }
}

// ---------------------------------------------------------------------------
// write_graph (file path)
// ---------------------------------------------------------------------------

void write_graph(std::string_view path, const graph &g,
                 const std::string &comment) {
  std::string path_str(path);
  std::ofstream file(path_str);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path_str);
  }
  write_graph(file, g, comment);
}

} // namespace teddy::graphrel
