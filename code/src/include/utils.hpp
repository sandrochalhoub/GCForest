
#ifndef _BLOSSOM_UTILS_HPP
#define _BLOSSOM_UTILS_HPP

#define INFTY(type) static_cast<type>(numeric_limits<type>::max()/2)

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <math.h>

// #define DEBUG_MODE

#define PRINTTRACE (options.verbosity >= DTOptions::YACKING) // and search_size % 100000 < 10)
#define REPRINTTRACE (algo.options.verbosity >= DTOptions::YACKING)

namespace blossom {

int log2_64(uint64_t value);

double cpu_time(void);
//
//
// double t{
//     static_cast<double>(static_cast<int>(100.0 * (cpu_time() - start_time)))
//     /
//     100.0};

template<typename floating_point>
floating_point fixedwidthfloat(const floating_point f, const int width) {
  // auto l{std::max(0,static_cast<int>(log10(f)))};
  //
  // if(l>=width)
  // 	return static_cast<floating_point>(static_cast<long>(f));
  //
  // floating_point m{pow(10.,width-l)};

  floating_point m{pow(10., std::max(width, static_cast<int>(log10(f))))};

  auto i{static_cast<long>(f * m)};

  auto v{static_cast<floating_point>(i) / m};

  // std::cout << i << " -> " << static_cast<floating_point>(i) << " -> " << v
  // << std::endl;

  return v;
}

}

#endif // _BLOSSOM_UTILS_HPP
