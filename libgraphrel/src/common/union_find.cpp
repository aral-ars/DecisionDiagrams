/**
 * @file union_find.cpp
 * @brief Union-Find (Disjoint Set Union) implementation
 *
 * Implements union-find with path compression and union by rank for efficient
 * connected component tracking. Used during edge contractions to merge vertices
 * and track connectivity for boundary set partition computation.
 *
 * Time Complexity: O(alpha(n)) amortized per operation, where alpha is the inverse
 * Ackermann function (effectively constant for practical purposes).
 */

#include "common/union_find.hpp"
#include <algorithm>
#include <stdexcept>

namespace teddy::graphrel {

union_find::union_find(std::size_t n) : parent_(n), rank_(n, 0) {
  if (n > 65535) {
    throw std::invalid_argument(
        "union_find: maximum 65535 elements (got " + std::to_string(n) + ")");
  }
  // Initially, each element is its own parent
  for (std::size_t i = 0; i < n; ++i) {
    parent_[i] = static_cast<uint16_t>(i);
  }
}

std::size_t union_find::find(std::size_t x) {
  /**
   * @brief Find root with path compression
   *
   * Path compression optimization: during the find operation, we make all
   * nodes on the path point directly to the root. This flattens the tree
   * and makes future finds faster.
   *
   * Example: If we have path x -> a -> b -> root, after find(x):
   *          x -> root, a -> root, b -> root (all point directly to root)
   */
  if (x >= parent_.size()) {
    throw std::out_of_range("union_find::find: index out of range");
  }

  // Path compression: make parent point directly to root
  // Recursively find root and update parent pointers along the path
  if (parent_[x] != static_cast<uint16_t>(x)) {
    parent_[x] = static_cast<uint16_t>(find(parent_[x]));
  }
  return parent_[x];
}

std::size_t union_find::find_root(std::size_t x) const {
  if (x >= parent_.size()) {
    throw std::out_of_range("union_find::find_root: index out of range");
  }

  // Find root without modification (no path compression)
  std::size_t root = x;
  while (parent_[root] != root) {
    root = parent_[root];
  }
  return root;
}

void union_find::unite(std::size_t x, std::size_t y) {
  /**
   * @brief Union two components using union by rank
   *
   * Union by rank optimization: always attach the smaller tree to the
   * larger tree's root. This keeps trees balanced and maintains O(log n)
   * height, which combined with path compression gives O(alpha(n)) amortized.
   *
   * Rank is an approximation of tree height. When ranks are equal, we
   * increment the rank of the new root.
   */
  if (x >= parent_.size() || y >= parent_.size()) {
    throw std::out_of_range("union_find::unite: index out of range");
  }

  // Find roots of both elements
  std::size_t root_x = find(x);
  std::size_t root_y = find(y);

  // Already in same component: nothing to do
  if (root_x == root_y) {
    return;
  }

  // Union by rank: attach smaller tree to larger tree
  // This keeps trees balanced and maintains logarithmic height
  if (rank_[root_x] < rank_[root_y]) {
    // Attach x's tree to y's root
    parent_[root_x] = static_cast<uint16_t>(root_y);
  } else if (rank_[root_x] > rank_[root_y]) {
    // Attach y's tree to x's root
    parent_[root_y] = static_cast<uint16_t>(root_x);
  } else {
    // Equal rank: attach y to x and increment x's rank
    // When ranks are equal, the resulting tree has height +1
    parent_[root_y] = static_cast<uint16_t>(root_x);
    rank_[root_x]++;
  }
}

std::vector<std::vector<std::size_t>> union_find::get_classes() const {
  std::vector<std::vector<std::size_t>> classes;

  // Map root -> list of elements
  std::vector<std::vector<std::size_t>> root_map(parent_.size());

  // Find root for each element (without modifying structure)
  for (std::size_t i = 0; i < parent_.size(); ++i) {
    std::size_t root = i;
    while (parent_[root] != root) {
      root = parent_[root];
    }
    root_map[root].push_back(i);
  }

  // Extract non-empty classes
  for (const auto &class_list : root_map) {
    if (!class_list.empty()) {
      classes.push_back(class_list);
    }
  }

  return classes;
}

void union_find::reset() {
  std::size_t n = parent_.size();
  for (std::size_t i = 0; i < n; ++i) {
    parent_[i] = static_cast<uint16_t>(i);
    rank_[i] = 0;
  }
}

} // namespace teddy::graphrel
