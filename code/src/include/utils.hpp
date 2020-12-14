


#ifndef _PRIMER_UTILS_HPP
#define _PRIMER_UTILS_HPP

#define INFTY(type) static_cast<type>(numeric_limits<type>::max()/2)

#include <cstdint>

// #define DEBUG_MODE

#define PRINTTRACE (options.verbosity >= DTOptions::YACKING) // and search_size % 100000 < 10)
#define REPRINTTRACE (algo.options.verbosity >= DTOptions::YACKING)

namespace blossom {

int log2_64(uint64_t value);

double cpu_time(void);
}

#endif // _PRIMER_UTILS_HPP
