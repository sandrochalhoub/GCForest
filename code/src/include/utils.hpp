


#ifndef _PRIMER_UTILS_HPP
#define _PRIMER_UTILS_HPP

#define INFTY(type) static_cast<type>(numeric_limits<type>::max())

// #define DEBUG_MODE

#define PRINTTRACE                                                             \
  (options.verbosity >= DTOptions::YACKING and search_size > 900 and           \
   search_size < 1000)

namespace primer {

	double cpu_time(void);

}

#endif // _PRIMER_UTILS_HPP
