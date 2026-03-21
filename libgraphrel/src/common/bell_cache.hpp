#ifndef GRAPHREL_SRC_BELL_CACHE_HPP
#define GRAPHREL_SRC_BELL_CACHE_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @file bell_cache.hpp
 * @brief Precomputed Stirling numbers of the second kind and Bell numbers
 *
 * Precomputes S(n,k) and B(n) for efficient partition canonicalization.
 * Used by partition_to_number and number_to_partition algorithms.
 *
 * Stirling numbers of the second kind S(n,k): number of ways to partition
 * n elements into k non-empty subsets.
 *
 * Bell numbers B(n): total number of partitions of n elements.
 */

namespace teddy::graphrel {

/**
 * @class bell_cache
 * @brief Cache for precomputed Stirling and Bell numbers
 *
 * Precomputes values up to max_n (typically 20) for O(1) lookup.
 * Uses dynamic programming with recurrence relations:
 * - S(n,k) = k*S(n-1,k) + S(n-1,k-1)
 * - B(n) = sum_{k=1}^n S(n,k)
 */
class bell_cache {
private:
  std::vector<std::vector<std::uint64_t>>
      stirling_;                    ///< stirling_[n][k] = S(n,k)
  std::vector<std::uint64_t> bell_; ///< bell_[n] = B(n)
  std::size_t max_n_;               ///< Maximum precomputed n

public:
  /**
   * @brief Constructor - precomputes all values
   * @param max_n Maximum n to precompute (typically 20)
   */
  explicit bell_cache(std::size_t max_n = 20);

  /**
   * @brief Get Stirling number of the second kind S(n,k)
   * @param n Number of elements
   * @param k Number of blocks
   * @return S(n,k) = number of ways to partition n elements into k blocks
   *
   * Returns 0 if n < k or if n > max_n (not precomputed)
   */
  std::uint64_t stirling(std::size_t n, std::size_t k) const;

  /**
   * @brief Get Bell number B(n)
   * @param n Number of elements
   * @return B(n) = total number of partitions of n elements
   *
   * Returns 0 if n > max_n (not precomputed)
   */
  std::uint64_t bell(std::size_t n) const;

  /**
   * @brief Get maximum precomputed n
   * @return max_n
   */
  std::size_t max_n() const { return max_n_; }

  /**
   * @brief Check if n is within precomputed range
   * @param n Value to check
   * @return true if n <= max_n
   */
  bool is_valid(std::size_t n) const { return n <= max_n_; }
};

} // namespace teddy::graphrel

#endif // GRAPHREL_SRC_BELL_CACHE_HPP
