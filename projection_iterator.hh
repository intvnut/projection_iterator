// Author:  Joe Zbiciak <joe.zbiciak@leftturnonly.info>
// SPDX-License-Identifier:  CC-BY-SA-4.0
#ifndef PROJECTION_ITERATOR_HH_
#define PROJECTION_ITERATOR_HH_

static_assert(__cplusplus, "Requires C++11 or newer.");

// C++11 has a much more limited form of constexpr that we cannot make use of
// in many places.  So for C++11, just remove the constexpr.
#if __cplusplus < 201400L
# define CONSTEXPR_AS_OF_CXX14 
#else
# define CONSTEXPR_AS_OF_CXX14 constexpr
#endif

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace jz {

// Implements an iterator that applies permutes the view of an indexed container
// by applying a projection to the index.  The base iterator currently must be a
// random access iterator.  The returned iterator is also a random access
// iterator.
//
// The projection() callable must accept an index of type ptrdiff_t relative to
// base, and return the projected index as an integral type.  It's up to the
// caller to provide a projection function whose returned values stay within
// the container relative to the base iterator.
//
// The make_projection_iterator() function below can deduce the types for Iter
// and Callable.  That's useful for pre-C++17 environments.  C++17 onward deduce
// template arguments from the constructor directly.
//
// Eventually, this may support other iterator categories such as bidirectional 
// and forward iterators; howeever, they will not meet the usual iterator
// performance guarantees for their category unless this iterator carries
// additional secondary storage.
template <typename Iter, typename Callable>
class ProjIter {
  using ptrdiff_t = std::ptrdiff_t;

  static_assert(
    std::is_base_of<
      std::random_access_iterator_tag,
      typename std::iterator_traits<Iter>::iterator_category>::value,
      "Must use a random access iterator.");

#if __cplusplus >= 201703L
  static_assert(
    std::is_integral<std::invoke_result_t<Callable, ptrdiff_t>>::value,
    "Projection callable must return an integral type.");
#elif __cplusplus >= 201100L
  static_assert(
    std::is_integral<typename std::result_of<Callable(ptrdiff_t)>::type>::value,
    "Projection callable must return an integral type.");
#endif

  using IterTraits        = typename std::iterator_traits<Iter>;

 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type   = typename IterTraits::difference_type;
  using value_type        = typename IterTraits::value_type;
  using pointer           = typename IterTraits::pointer;
  using reference         = typename IterTraits::reference;

  explicit constexpr ProjIter(Iter base, Callable projection) noexcept
  : base_{std::move(base)}, projection_{std::move(projection)}, index_{0} {}

  ProjIter(const ProjIter&) = default;
  ProjIter(ProjIter&&) = default;
  ~ProjIter() = default;

  // For copy and move assignment, assume base and projection are the
  // same for both.  It's undefined behavior if it's not.
  ProjIter& operator=(const ProjIter& rhs) noexcept {
    base_  = rhs.base_;
    index_ = rhs.index_;
    return *this;
  }
  ProjIter& operator=(ProjIter&& rhs) noexcept {
    base_  = rhs.base_;
    index_ = rhs.index_;
    return *this;
  } 

  // Dereference returns an lvalue reference to the element.
  CONSTEXPR_AS_OF_CXX14 reference operator*() const noexcept {
    return *projected_();
  }

  // Returns the original iterator offset to the correct point.
  CONSTEXPR_AS_OF_CXX14 value_type operator->() const noexcept {
    return projected_();
  }

  // Increments our index relative to first.
  CONSTEXPR_AS_OF_CXX14 ProjIter& operator++() noexcept {
    ++index_;
    return *this;
  }

  CONSTEXPR_AS_OF_CXX14 ProjIter operator++(int) noexcept {
    auto copy = *this;
    this->operator++();
    return copy;
  }

  // Decrements our index relative to first.
  CONSTEXPR_AS_OF_CXX14 ProjIter& operator--() noexcept {
    --index_;
    return *this;
  }

  CONSTEXPR_AS_OF_CXX14 ProjIter operator--(int) noexcept {
    auto copy = *this;
    this->operator--();
    return copy;
  }

  // Adds to our iterator.
  CONSTEXPR_AS_OF_CXX14 ProjIter& operator+=(const ptrdiff_t delta) noexcept {
    index_ += delta;
    return *this;
  }

  CONSTEXPR_AS_OF_CXX14  ProjIter operator+(const ptrdiff_t delta)
      const noexcept {
    auto copy = *this;
    copy += delta;
    return copy;
  }

  // Subtracts from our iterator.
  CONSTEXPR_AS_OF_CXX14 ProjIter& operator-=(const ptrdiff_t delta) noexcept {
    index_ += delta;
    return *this;
  }

  CONSTEXPR_AS_OF_CXX14 ProjIter operator-(const ptrdiff_t delta)
      const noexcept {
    auto copy = *this;
    copy -= delta;
    return copy;
  }

  // Returns difference between two iterators.
  constexpr ptrdiff_t operator-(const ProjIter& rhs) const noexcept {
    return index_ - rhs.index_;
  }

  // Base comparisons.  Note, these don't check whether you're comparing
  // dissimilar iterators (e.g. different base iterators).
  constexpr bool operator<(const ProjIter& rhs) const noexcept {
    return index_ < rhs.index_;
  }

  constexpr bool operator==(const ProjIter& rhs) const noexcept {
    return index_ == rhs.index_;
  }

  // Derived comparisons.
  constexpr bool operator>(const ProjIter& rhs) const noexcept {
    return rhs < *this;
  }

  constexpr bool operator<=(const ProjIter& rhs) const noexcept {
    return !(*this > rhs);
  }

  constexpr bool operator>=(const ProjIter& rhs) const noexcept {
    return !(*this < rhs);
  }

  constexpr bool operator!=(const ProjIter& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  Iter base_;
  Callable projection_;
  ptrdiff_t index_;

  // Returns the advanced pointer.  While we could do this with a simple
  // addition, std::advance opens the possibility we include other iterator
  // categories eventually.
  CONSTEXPR_AS_OF_CXX14 Iter projected_() const {
    Iter copy = base_;
    std::advance(copy, projection_(index_));
    return copy;
  }
};

// Provides template argument deduction for ProjIter for C++11 and C++14.
// C++17 onward can deduce the template parameters from the constructor.
template <typename Iter, typename Callable>
constexpr inline auto make_projection_iterator(
    Iter base, Callable projection) noexcept
    -> ProjIter<Iter, Callable> {
  return ProjIter<Iter, Callable>(base, std::move(projection));
}

}  // namespace jz

#undef CONSTEXPR_AS_OF_CXX14

#endif // PROJECTION_ITERATOR_HH_
