#ifndef INCLUDE__PTHREAD_NP_SHIM_HPP
#define INCLUDE__PTHREAD_NP_SHIM_HPP

#ifdef __FreeBSD__
	#include <pthread_np.h>
#else
	#include <pthread.h>

	int pthread_getthreadid_np(void);
	void pthread_set_name_np(pthread_t tid, const char *name);
#endif

#endif
