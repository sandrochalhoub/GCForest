
#ifndef _BLOSSOM_UTILS_HPP
#define _BLOSSOM_UTILS_HPP

#define INFTY(type) static_cast<type>(numeric_limits<type>::max()/2)

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <math.h>

// #define DEBUG_MODE

#define PRINTTRACE                                                             \
  (options.verbosity >= DTOptions::YACKING)
// #define REPRINTTRACE (algo.options.verbosity >= DTOptions::YACKING)

namespace blossom {

int log2_64(uint64_t value);

double cpu_time(void);

template<typename floating_point>
floating_point fixedwidthfloat(const floating_point f, const int width) {
  floating_point m{pow(10., std::max(width, static_cast<int>(log10(f))))};
  auto i{static_cast<long>(f * m)};
  return (static_cast<floating_point>(i) / m);
}

}

#endif // _BLOSSOM_UTILS_HPP
