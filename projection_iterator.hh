// Author:  Joe Zbiciak <joe.zbiciak@leftturnonly.info>
// SPDX-License-Identifier:  CC-BY-SA-4.0
#ifndef PROJECTION_ITERATOR_HH_
#define PROJECTION_ITERATOR_HH_

#if __cplusplus < 201400L
# error "Requires C++14 or newer."
#endif

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace jz {

// Returns an iterator representing a projection applied to the container that
// the provided base iterator points into.  The base iterator must be a random
// access iterator.  The returned iterator is also a random access iterator.
//
// The projection() callable must accept an index of type ptrdiff_t relative to
// base, and return the projected index as an integral type.  It's up to the
// caller to provide a projection function whose returned values stay within
// the container relative to the base iterator.
template <typename Iter, typename Callable>
constexpr inline auto make_projection_iterator(
    Iter base, Callable& projection) noexcept {
  using std::ptrdiff_t;

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
  using IterTraits = typename std::iterator_traits<Iter>;

  // The projection iterator.  Calls projection(index) to project a virtual
  // index into an actual index applied to the base iterator.
  class ProjIter {
   public:
    explicit constexpr ProjIter(Iter base, Callable& projection) noexcept
    : base_{std::move(base)}, projection_{&projection}, index_{0} {}

    ProjIter(const ProjIter&) = default;
    ProjIter(ProjIter&&) = default;
    ProjIter& operator=(const ProjIter&) = default;
    ProjIter& operator=(ProjIter&&) = default;
    ~ProjIter() = default;

    // Dereference returns an lvalue reference to the element.
    constexpr auto& operator*() const noexcept {
      return *projected_();
    }

    // Returns the original iterator offset to the correct point.
    constexpr auto operator->() const noexcept {
      return projected_();
    }

    // Increments our index relative to first.
    constexpr auto& operator++() noexcept {
      ++index_;
      return *this;
    }

    constexpr auto operator++(int) noexcept {
      auto copy = *this;
      this->operator++();
      return copy;
    }

    // Decrements our index relative to first.
    constexpr auto& operator--() noexcept {
      --index_;
      return *this;
    }

    constexpr auto operator--(int) noexcept {
      auto copy = *this;
      this->operator--();
      return copy;
    }

    // Adds to our iterator.
    constexpr auto& operator+=(const ptrdiff_t delta) noexcept {
      index_ += delta;
      return *this;
    }

    constexpr auto operator+(const ptrdiff_t delta) const noexcept {
      auto copy = *this;
      copy += delta;
      return copy;
    }

    // Subtracts from our iterator.
    constexpr auto& operator-=(const ptrdiff_t delta) noexcept {
      index_ += delta;
      return *this;
    }

    constexpr auto operator-(const ptrdiff_t delta) const noexcept {
      auto copy = *this;
      copy -= delta;
      return copy;
    }

    // Returns difference between two iterators.
    constexpr auto operator-(const ProjIter& rhs) const noexcept {
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

    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = typename IterTraits::difference_type;
    using value_type        = typename IterTraits::value_type;
    using pointer           = typename IterTraits::pointer;
    using reference         = typename IterTraits::reference;

   private:
    Iter base_;
    Callable* projection_;
    ptrdiff_t index_;

    // Returns the advanced pointer.  While we could do this with a simple
    // addition, std::advance opens the possibility we include other iterator
    // categories eventually.
    constexpr Iter projected_() const {
      Iter copy = base_;
      std::advance(copy, (*projection_)(index_));
      return copy;
    }

    // Suppresses meaningless warnings about these types not being used.
    // They are actually used, but G++'s warning gets it wrong.
    void unused_(iterator_category, difference_type, value_type, pointer,
                 reference) {}
  };

  return ProjIter(base, projection);
}

}  // namespace jz

#endif // PROJECTION_ITERATOR_HH_
