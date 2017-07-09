#include "pthread_np_shim.hpp"


#ifndef __FreeBSD__

	#include <syscall.h>
	#include <unistd.h>


	int pthread_getthreadid_np(void) {
		return syscall(SYS_gettid);
	}


	void pthread_set_name_np(pthread_t tid, const char *name) {
		pthread_setname_np(tid, name);
	}

#endif

