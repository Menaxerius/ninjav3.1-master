#ifndef INCLUDE__TIMESTAMPS_HPP
#define INCLUDE__TIMESTAMPS_HPP

#include <sys/time.h>

#define TV_LT(a,b) ( (a).tv_sec < (b).tv_sec || ( (a).tv_sec == (b).tv_sec && (a).tv_usec < (b).tv_usec ) )
#define TV_LE(a,b) ( (a).tv_sec <= (b).tv_sec || ( (a).tv_sec == (b).tv_sec && (a).tv_usec <= (b).tv_usec ) )

#define TV_GT(a,b) ( (a).tv_sec > (b).tv_sec || ( (a).tv_sec == (b).tv_sec && (a).tv_usec > (b).tv_usec ) )
#define TV_GE(a,b) ( (a).tv_sec >= (b).tv_sec || ( (a).tv_sec == (b).tv_sec && (a).tv_usec >= (b).tv_usec ) )

#define TV_ADD(a,b) (a).tv_sec += (b).tv_sec; (a).tv_usec += (b).tv_usec; if ( (a).tv_usec > 1000000UL ) { (a).tv_sec++; (a).tv_usec -= 1000000UL; }

#define TV_NONZERO(a) ( (a).tv_sec != 0 && (a).tv_usec != 0 )
#define TV_ISZERO(a) ( (a).tv_sec == 0 && (a).tv_usec == 0 )

#endif
