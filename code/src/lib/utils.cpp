
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include "utils.hpp"


namespace primer {

	double cpu_time(void) {
	  struct rusage ru;
	  getrusage(RUSAGE_SELF, &ru);
	  return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
	}

}