


#ifndef _PRIMER_UTILS_HPP
#define _PRIMER_UTILS_HPP

#define INFTY static_cast<size_t>(numeric_limits<size_t>::max())

// #define DEBUG_MODE

#define PRINTTRACE (options.verbosity >= DTOptions::YACKING)

namespace primer {

	double cpu_time(void);

}

#endif // _PRIMER_UTILS_HPP
