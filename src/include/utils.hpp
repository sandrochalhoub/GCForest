
#ifndef _BLOSSOM_UTILS_HPP
#define _BLOSSOM_UTILS_HPP

#define INFTY(type) static_cast<type>(numeric_limits<type>::max()/2)

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <cmath>

// #define DEBUG_MODE

// #define PRINTTRACE                                                             \
//   (options.verbosity > DTOptions::YACKING)
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

#define FLOAT_PRECISION static_cast<T>(1.e-6)

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
equal(const T &a, const T &b) {
  return a == b;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
lt(const T &a, const T &b) {
  return a < b;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
equal(const T &a, const T &b) {
  return std::fabs(a - b) < FLOAT_PRECISION;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
lt(const T &a, const T &b) {
  return a + FLOAT_PRECISION < b;
}

}

#endif // _BLOSSOM_UTILS_HPP
