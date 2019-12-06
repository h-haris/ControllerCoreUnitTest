
#include <Carbon/Carbon.h>

/*
 *  Copyright © 1997-2002 Metrowerks Corporation.  All Rights Reserved.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */


//#include <Quesa/Quesa.h>
//#include <Quesa/QuesaErrors.h>

#include "uniBench.h"

int main(void)
{
/*	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);*/
	
	InitCursor();
	
	do_bench();
	
	return 0;
}