/*
 *  uniBench.h
 *  QuesaControllerBenchX
 *
 *  Created by Ole Hartmann on Wed Oct 08 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef BenchX
	#include <Carbon/Carbon.h>
#else
	typedef unsigned long                           TQ3Uns32;
	typedef signed long                             TQ3Int32;
#endif

char * do_bench(void);