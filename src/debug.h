/*
 * Simple debug
 */


#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

/* force debug */
#define DEBUG_IQRF_DEV 0


#ifndef DEBUG_PREPEND
	#define DEBUG_PREPEND "[debug] "
#endif


#if DEBUG_IQRF_DEV
	#define FDBG(f, fmt, args...) \
		fprintf(f, DEBUG_PREPEND fmt , ##args);
#else
	#define FDBG(f, fmt, args...) {}
#endif

#define DBG(fmt, args...) FDBG(stdout, fmt , ##args)
#define ERR(fmt, args...) do { \
	FDBG(stderr, "**fatal: " fmt , ##args);\
	abort();\
} while(0)
	


#endif // DEBUG_H
