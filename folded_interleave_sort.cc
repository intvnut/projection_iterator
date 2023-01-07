// Author:  Joe Zbiciak <joe.zbiciak@leftturnonly.info>
// SPDX-License-Identifier:  CC-BY-SA-4.0
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <random>

#include "projection_iterator.hh"

namespace {

using std::ptrdiff_t;
using jz::make_projection_iterator;

// Returns a projection function for a particular size random-access container
// such that this view through the projection:
//
//      0, 1, 2, 3, 4, 5, 6, 7, 8, 9
// 
// is laid out in memory as:
// 
//      0, 9, 1, 8, 2, 7, 3, 6, 4, 5
auto make_folded_interleave_projection(ptrdiff_t size) {
  return [size](const ptrdiff_t index) {
    const auto index2 = 2 * index;
    return index2 >= size ? 2*size - index2 - 1 : index2; 
  };
}

// Prints a span of values on stdout, separated by commas.
template <typename Iter>
void print_span(Iter first, Iter last) {
  for (Iter it = first; it != last; ++it) {
    if (it != first) {
      std::cout << ", ";
    }
    std::cout << *it;
  }
}

}  // namespace

int main() {
  std::random_device rd;
  auto g = std::mt19937_64{rd()};
  auto v = std::vector<int>{};

  // Simple incrementing ranges in shuffled order.
  for (int i = 0; i < 15; ++i) {
    v.push_back(i);
    std::shuffle(v.begin(), v.end(), g);

    std::cout << "Before:   ";
    print_span(v.cbegin(), v.cend());
    std::cout << '\n';

    auto fip_proj  = make_folded_interleave_projection(v.size());
    auto fip_begin = make_projection_iterator(v.begin(), fip_proj);
    auto fip_end   = fip_begin + v.size();

    std::sort(fip_begin, fip_end);

    std::cout << "After:    ";
    print_span(v.cbegin(), v.cend());
    std::cout << '\n';

    std::cout << "FIP view: ";
    print_span(fip_begin, fip_end);
    std::cout << "\n\n";
  }

  // Random values in shuffled order.
  for (int i = 0; i < 10; ++i) {
    auto ui99 = std::uniform_int_distribution<>(0, 99);
    for (auto& elem : v) {
      elem = ui99(g);
    }
    std::shuffle(v.begin(), v.end(), g);

    std::cout << "Before:   ";
    print_span(v.cbegin(), v.cend());
    std::cout << '\n';

    auto fip_proj  = make_folded_interleave_projection(v.size());
    auto fip_begin = make_projection_iterator(v.begin(), fip_proj);
    auto fip_end   = fip_begin + v.size();

    std::sort(fip_begin, fip_end);

    std::cout << "After:    ";
    print_span(v.cbegin(), v.cend());
    std::cout << '\n';

    std::cout << "FIP view: ";
    print_span(fip_begin, fip_end);
    std::cout << "\n\n";
  }

  // Again, but with one less value (to test even/odd length).
  v.pop_back();
  for (int i = 0; i < 10; ++i) {
    auto ui99 = std::uniform_int_distribution<>(0, 99);
    for (auto& elem : v) {
      elem = ui99(g);
    }
    std::shuffle(v.begin(), v.end(), g);

    std::cout << "Before:   ";
    print_span(v.cbegin(), v.cend());
    std::cout << '\n';

    auto fip_proj  = make_folded_interleave_projection(v.size());
    auto fip_begin = make_projection_iterator(v.begin(), fip_proj);
    auto fip_end   = fip_begin + v.size();

    std::sort(fip_begin, fip_end);

    std::cout << "After:    ";
    print_span(v.cbegin(), v.cend());
    std::cout << '\n';

    std::cout << "FIP view: ";
    print_span(fip_begin, fip_end);
    std::cout << "\n\n";
  }
}
