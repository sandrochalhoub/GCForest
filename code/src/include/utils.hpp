


#ifndef _PRIMER_UTILS_HPP
#define _PRIMER_UTILS_HPP

#define INFTY(type) static_cast<type>(numeric_limits<type>::max()/2)

// #define DEBUG_MODE

#define PRINTTRACE (options.verbosity >= DTOptions::YACKING)

namespace primer {

	double cpu_time(void);

}

#endif // _PRIMER_UTILS_HPP
