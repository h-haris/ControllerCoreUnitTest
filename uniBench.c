/*
 *  uniBench.c
 *  QuesaControllerBenchX
 *
 *  Created by Ole Hartmann on Wed Oct 08 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include <math.h>
#ifdef BenchX
//for Quesa
#ifndef QUESA_OS_MACINTOSH
#define QUESA_OS_MACINTOSH 1
#endif

#ifndef Q3_DEBUG
#define Q3_DEBUG 1
#endif

#include <Quesa/Quesa.h>
//#include "Quesa/Quesa.h"
#include <Quesa/QuesaController.h>
#include <Quesa/QuesaMath.h>
#else
#include <QD3D.h>
#include <QD3DController.h>
#include <QD3DMath.h>
#include <Sound.h>
#include <stdio.h>
#endif

#include "uniBench.h"

//unit testing
#include "minunit.h"

#define FL_EPSILON (0.00001)
//#define FL_EQUAL(FL_result,FL_expectedResult) (fabsf( (FL_result)-(FL_expectedResult) ) < FL_EPSILON)
#define FL_EQUAL(FL_result,FL_expectedResult) (fabs( (FL_result)-(FL_expectedResult) ) < FL_EPSILON)

int tests_run = 0; //must be int as used by minunit.h macros


/* debug and dbginfo based on: http://c-faq.com/cpp/debugmacs.html . Used its DEBUG macro code to define mu_dbg: */
#define mu_dbg dbginfo(__LINE__, __FILE__), debug

void debug(const char *, ...);
void dbginfo(int, const char *);

static const char *dbgfile;
static int dbgline;

void dbginfo(int line, const char *file)
{
	dbgfile = file;
	dbgline = line;//TODO: set and activate breakpoint here if mu_assert fails
};

void debug(const char *fmt, ...)
{
    char _debugstring[256];
    char _vstring[200];
    va_list argp;
    //sprintf(_debugstring, "MU_ASSERT: \"%s\", line %d: ", dbgfile, dbgline);
	va_start(argp, fmt);
	vsprintf(_vstring, fmt, argp);
	va_end(argp);
	//sprintf(_debugstring, "%s\n", _debugstring);
	sprintf(_debugstring, "MU_ASSERT: \"%s\", line %d. %s\n", dbgfile, dbgline, _vstring);

#ifdef __MWERKS__
	debugstr(_debugstring);
#else
	fprintf(stderr, "%s\n",_debugstring);
#endif	
	
};


/*****************************************
 * globals used and filled by unit tests
 *****************************************/

TQ3Boolean			controllerListChanged;
TQ3Uns32			controllerListSerialNumber;
TQ3ControllerData	ControllerData;
TQ3ControllerStateObject
ControllerState;
TQ3Status			Status3D;
TQ3ControllerRef	tempController = (TQ3ControllerRef)-1;
TQ3ControllerRef	ClientController;
TQ3ControllerRef	DriverController;
TQ3Boolean			controllerActivation;

TQ3TrackerObject	ClientTracker;
TQ3Boolean			trackerActivation;
float 				posTresh;
float 				oriTresh;

char				tempString[250];
TQ3Uns32			tempUns32;
TQ3Uns32			TrackerSerNum,CtrlSerNum;
TQ3Boolean			tempBool,changed;
TQ3Point3D			position;
TQ3Vector3D			deltaPos;
TQ3Quaternion		orientation;
TQ3Quaternion		deltaOri;
float				values[2];
TQ3Uns32			whatSet,whatGet,whatSize;

//callback functions
TQ3Uns32	lastsize, what0, what1, what_work, trackerProcCalled, doBeep;

//controller_GetChannel
TQ3Status ChannelGetMethod(	TQ3ControllerRef    controllerRef,
                            TQ3Uns32            channel,
                            void                *data,
                            TQ3Uns32            *dataSize);

TQ3Status ChannelGetMethod(	TQ3ControllerRef    controllerRef,
                            TQ3Uns32            channel,
                            void                *data,
                            TQ3Uns32            *dataSize)
{
	TQ3Status	status = kQ3Success;
	//what0 and what1 crossed!!
	switch(channel)
	{
		case 0:
			*(TQ3Uns32*)data=what1;
			break;
		case 1:
			*(TQ3Uns32*)data=what0;
			break;
		case 2:
			*(TQ3Uns32*)data=lastsize;
			break;
		case 3:
			*(TQ3Uns32*)data=what_work;
            status = kQ3Failure;
			break;
		default:
			*(TQ3Uns32*)data=65000;
			status = kQ3Failure;
			break;
	}
    
	*dataSize=sizeof(TQ3Uns32);
	
	return status;
};

//controller_SetChannel
TQ3Status ChannelSetMethod( TQ3ControllerRef    controllerRef,
                            TQ3Uns32            channel,
                            const void          *data,//NULL is valid!
                            TQ3Uns32            dataSize);

TQ3Status ChannelSetMethod( TQ3ControllerRef    controllerRef,
                            TQ3Uns32            channel,
                            const void          *data,//NULL is valid!
                            TQ3Uns32            dataSize)
{
	//what
	switch(channel)
	{
		case 0:
			if (data==NULL)
				what0=(TQ3Uns32)NULL;
			else
				what0=*(TQ3Uns32*)data;
			break;
		case 1:
			if (data==NULL)
				what1=(TQ3Uns32)NULL;
			else
				what1=*(TQ3Uns32*)data;
			break;
		case 3:
			if (data==NULL)
				what_work=(TQ3Uns32)NULL;
			else
				what_work=*(TQ3Uns32*)data;
			break;
	}
	lastsize=dataSize;
	
	return kQ3Success;//kQ3Failure not implemented
};

//tracker_notification_proc
TQ3Status TrackerProc(TQ3TrackerObject trackerObject, TQ3ControllerRef controllerRef);

TQ3Status TrackerProc(TQ3TrackerObject trackerObject, TQ3ControllerRef controllerRef)
{
	//do what ever to place a breakpoint here
#if 1
    doBeep++;
#else
#ifdef BenchX
    NSBeep();
#else
	SysBeep(100);
#endif
#endif
    trackerProcCalled++;
    return kQ3Success;
};

static char * test_000(void)
{
    Status3D = Q3Initialize();//take a look at registration of TQ3TrackerObject!!
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
};

static char * test_001(void)
{
    Status3D = Q3Controller_Next(NULL,&ClientController);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, ClientController != NULL", ClientController == NULL);
    
    return 0;
};

static char * test_002(void)
{
    Status3D = Q3Controller_GetListChanged(&controllerListChanged,&controllerListSerialNumber);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#ifdef __MWERKS__
	mu_assert("error, controllerListChanged != kQ3False", controllerListChanged == kQ3True);
    mu_assert("error, controllerListSerialNumber != 1", controllerListSerialNumber == 1);
#else
	mu_assert("error, controllerListChanged != kQ3False", controllerListChanged == kQ3False);
    mu_assert("error, controllerListSerialNumber != 0", controllerListSerialNumber == 0);
#endif	
    return 0;
};

static char * test_003(void)
{
    Status3D = Q3Controller_Next(tempController,&ClientController);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, ClientController != NULL", ClientController == NULL);//should be NULL in OSX, too

    return 0;
};

static char * test_004(void)
{
    //Fill struct TQ3ControllerData for Controller Creation
    ControllerData.signature="com.HMDP.BenchController\0";
    ControllerData.valueCount=16;//max valueCount is unknown for QD3D! seems to be unlimited (4096 worked)
    ControllerData.channelCount=4;
    ControllerData.channelGetMethod=ChannelGetMethod;
    ControllerData.channelSetMethod=ChannelSetMethod;
    DriverController = Q3Controller_New(&ControllerData);
    mu_assert("error, DriverController == NULL", DriverController != NULL);
    
    return 0;
};

static char * test_005(void)
{
    Status3D = Q3Controller_GetListChanged(&controllerListChanged,&controllerListSerialNumber);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not controllerListChanged == kQ3True", controllerListChanged == kQ3True);
//controllerListSerialNumber = (QD3D: 2; Quesa: 1)
#ifdef __MWERKS__
    mu_assert("error, not controllerListSerialNumber == 2", controllerListSerialNumber == 2);
#else
    mu_assert("error, not controllerListSerialNumber == 1", controllerListSerialNumber == 1);
#endif	
    return 0;
};

static char * test_006(void)
{
    Status3D = Q3Controller_Next(NULL,&ClientController);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#ifdef __MWERKS__
    mu_assert("error, not ClientController == DriverController", ClientController == DriverController);
#else
    mu_assert("error, not ClientController == DriverController",
              kCFCompareEqualTo == CFStringCompare((CFStringRef)ClientController, (CFStringRef)DriverController,
                                                   kCFCompareCaseInsensitive));
#endif
    return 0;
};

static char * test_007(void)
{
    ControllerState = Q3ControllerState_New (ClientController);
    mu_assert("error, ControllerState == NULL", ControllerState != NULL);
    
    return 0;
};

static char * test_008(void)
{
    Status3D = Q3Controller_Next(ClientController,&tempController);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    /*
     tempController :
     QD3D:  != NULL;
     Quesa: Should be NULL in OSX - no System Cursor?!
     */
    mu_assert("error, still another controller available", tempController == NULL);
    
    return 0;
};

static char * test_009(void)
{
    //New Tracker
    ClientTracker = Q3Tracker_New(TrackerProc);
    mu_assert("error, not ClientTracker != NULL", ClientTracker != NULL);
    
    return 0;
};

static char * test_010(void)
{
    Status3D = Q3Tracker_GetNotifyThresholds(ClientTracker,&posTresh,&oriTresh);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not posTresh == 0.0", posTresh == 0.0);
    mu_assert("error, not oriTresh == 0.0", oriTresh == 0.0);
    
    posTresh = 0.1;
    oriTresh = 0.1;
    Status3D = Q3Tracker_SetNotifyThresholds(ClientTracker,posTresh,oriTresh);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Tracker_GetNotifyThresholds(ClientTracker,&posTresh,&oriTresh);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not posTresh == 0.1", FL_EQUAL(posTresh, 0.1));
    mu_assert("error, not oriTresh == 0.1", FL_EQUAL(oriTresh, 0.1));
    
    posTresh = 0.0;
    oriTresh = 0.0;
    Status3D = Q3Tracker_SetNotifyThresholds(ClientTracker,posTresh,oriTresh);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
};

static char * test_011(void)
{
    //deactivate Tracker
    Status3D = Q3Tracker_SetActivation(ClientTracker,kQ3False);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
};

static char * test_012(void)
{
    Status3D =  Q3Tracker_GetActivation(ClientTracker,&trackerActivation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not trackerActivation == kQ3False", trackerActivation == kQ3False);
    
    return 0;
};

static char * test_013(void)
{
    //indicates an active controller !and! an active tracker
    Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success); 
#ifdef __MWERKS__
    mu_assert("error, not tempBool == kQ3True. Controller has no associated tracker!", tempBool == kQ3True);
#else
    //Quesa: tempBool = kQFalse; no Tracker associated!
    mu_assert("error, not tempBool == kQ3False. Controller has an associated tracker!", tempBool == kQ3False);
#endif
    
    return 0;
};

static char * test_014(void)
{
    Status3D = Q3Controller_SetTracker(ClientController,ClientTracker);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
};

static char * test_015(void)
{
    Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not controllerActivation == kQ3True", controllerActivation == kQ3True);
    
    return 0;
};

static char * test_016(void)
{
    //inactive Tracker
    Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3False. Controller has an active tracker!", tempBool == kQ3False);
    
    return 0;
};

static char * test_017(void)
{
    //active Tracker
    Status3D = Q3Tracker_SetActivation(ClientTracker,kQ3True);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D =  Q3Tracker_GetActivation(ClientTracker,&trackerActivation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3True. Tracker is inactive!", trackerActivation == kQ3True);
    
    Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3True. Controller has an inactive tracker!", tempBool == kQ3True);
    
    return 0;
};

static char * test_018(void)
{
    Status3D = Q3Controller_GetSignature(ClientController,tempString,5);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong string returned", 0==strcmp("com.", tempString));
    
    Status3D = Q3Controller_GetSignature(ClientController,tempString,128);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong string returned", 0==strcmp("com.HMDP.BenchController", tempString));

    return 0;
}

static char * test_019(void)
{
    Status3D = Q3Controller_GetValueCount(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempUns32 == 16", tempUns32 == 16);
    
    return 0;
};

static char * test_020(void)
{
    Status3D = Q3Controller_Track2DCursor(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3False", tempBool == kQ3False);
    
    Status3D = Q3Controller_Track3DCursor(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3False", tempBool == kQ3False);
    
    return 0;
};

static char * test_021(void)
{
    TQ3Uns32 lastTrackerProcCalled = trackerProcCalled;
    Status3D = Q3Controller_SetButtons(DriverController,5);
    //Take a look at Q3Tracker_ChangeButtons, too
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc wasn't called as trackerProcCalled unchanged", lastTrackerProcCalled != trackerProcCalled);

    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 5);

    tempUns32 = 0;
    Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 5);

    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#if 1
    mu_assert("error, wrong buttons set", tempUns32 == 5);
#else
    mu_assert("error, buttons not cleared. Buttons were returned the call before, then the tracker set buttons bitfield to zero.", tempUns32 == 0);
#endif    
    return 0;
}

static char * test_022(void)
{
    TQ3Uns32 lastTrackerProcCalled;
    
    TrackerSerNum=0;
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,NULL,NULL,&TrackerSerNum);//TBC
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    mu_assert("error, not TrackerSerNum == 1", TrackerSerNum == 1);
    
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,10,NULL,NULL,NULL);
    mu_assert("error, no kQ3Failure", Status3D == kQ3Failure);
    
    tempUns32=5;
    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,10,tempUns32,&position,NULL);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    lastTrackerProcCalled = trackerProcCalled;
    position.x=1.;
    position.y=1.;
    position.z=1.;
    Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc wasn't called as trackerProcCalled unchanged", lastTrackerProcCalled != trackerProcCalled);

    return 0;
}

static char * test_023(void)
{
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);//TBC
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 1.)&&(position.y == 1.)&&(position.z == 1.) );
    mu_assert("error, wrong tracker deltaPos", (deltaPos.x == 0.)&&(deltaPos.y == 0.)&&(deltaPos.z == 0.) );
    mu_assert("error, not changed == kQ3True", changed == kQ3True);
    mu_assert("error, not TrackerSerNum == 2", TrackerSerNum == 2);

    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,20,(TQ3Uns32)NULL,&position,NULL);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
}

static char * test_024(void)
{
    Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong position", (position.x == 1.)&&(position.y == 1.)&&(position.z == 1.) );

    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 1.)&&(position.y == 1.)&&(position.z == 1.) );
    mu_assert("error, wrong tracker deltaPos", (deltaPos.x == 0.)&&(deltaPos.y == 0.)&&(deltaPos.z == 0.) );
    mu_assert("error, not changed == kQ3False", changed == kQ3False);
    mu_assert("error, not TrackerSerNum == 2", TrackerSerNum == 2);

    deltaPos.x=1.;
    deltaPos.y=1.;
    deltaPos.z=1.;
    Status3D = Q3Controller_MoveTrackerPosition(DriverController,&deltaPos);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong position", (position.x == 2.)&&(position.y == 2.)&&(position.z == 2.) );

    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);//TBC
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 2.)&&(position.y == 2.)&&(position.z == 2.) );
    mu_assert("error, wrong tracker deltaPos", (deltaPos.x == 0.)&&(deltaPos.y == 0.)&&(deltaPos.z == 0.) );
    //why no delta? Doc? Threshold? previously cleared?
    mu_assert("error, not changed == kQ3True", changed == kQ3True);
    mu_assert("error, not TrackerSerNum == 3", TrackerSerNum == 3);

    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,30,(TQ3Uns32)NULL,&position,NULL);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
}

static char * test_025(void)
{
    TrackerSerNum=0;
    Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,NULL,NULL,&TrackerSerNum);//TBC
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );//orientation = 1,-0,-0,-0
    mu_assert("error, not TrackerSerNum == 1", TrackerSerNum == 1);

    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,40,(TQ3Uns32)NULL,NULL,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    (void)Q3Quaternion_SetIdentity(&orientation);//orientation = 1,0,0,0;
    Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );//orientation = 1,-0,-0,-0

    Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);//TBC
	mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );
    mu_assert("error, wrong tracker deltaOri", (deltaOri.w == 1.)&&(deltaOri.x == 0)&&(deltaOri.y == 0)&&(deltaOri.z == 0) );//deltaOri = 1,-0,-0,-0;
    mu_assert("error, not changed == kQ3True", changed == kQ3True);
    mu_assert("error, not TrackerSerNum == 2", TrackerSerNum == 2);
    
    return 0;
}

static char * test_026(void)
{
    (void)Q3Quaternion_SetRotate_Y(&deltaOri,45);//deltaOri = -0.8733047,0,-0.4871745,0;
    Status3D = Q3Controller_MoveTrackerOrientation(DriverController,&deltaOri);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong orientation", FL_EQUAL(orientation.w, -0.8733047) && (orientation.x == 0.) && FL_EQUAL(orientation.y, -0.4871745) && (orientation.z == 0.) );
    //orientation = -0.8733047,-0,-0.4871745,-0;
    
    Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);//TBC
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", FL_EQUAL(orientation.w, -0.8733047) && (orientation.x == 0.) && FL_EQUAL(orientation.y, -0.4871745) && (orientation.z == 0.) );
    //orientation = -0.8733047,-0,-0.4871745,-0;
    mu_assert("error, wrong tracker deltaOri", (deltaOri.w == 1.)&&(deltaOri.x == 0)&&(deltaOri.y == 0)&&(deltaOri.z == 0) );//deltaOri = 1,0,0,0;
    mu_assert("error, not changed == kQ3True", changed == kQ3True);
    mu_assert("error, not TrackerSerNum == 3", TrackerSerNum == 3);

    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,50,(TQ3Uns32)NULL,NULL,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
}

static char * test_027(void)
{
    Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, unitialized values", (values[0] == 0.)&&(values[1] == 0.) );
    mu_assert("error, not CtrlSerNum == 1", CtrlSerNum == 1);

    values[0]= 1.0;
    values[1]=-1.0;
    Status3D = Q3Controller_SetValues(DriverController,values,2);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    Status3D = Q3Controller_GetValues(ClientController,2,values,&changed,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong values", (values[0] == 1.0)&&(values[1] == -1.0) );
    mu_assert("error, not CtrlSerNum == 2", CtrlSerNum == 2);
    
    return 0;
}

static char * test_028(void)
{
    whatSet=1500;
    Status3D = Q3Controller_SetChannel(ClientController,3,&whatSet,sizeof(whatSet));
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    return 0;
}

static char * test_029(void)
{
    //preparation for the test of Q3ControllerState_SaveAndReset
    position.x=0.;
    position.y=0.;
    position.z=0.;
    Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    (void)Q3Quaternion_SetIdentity(&orientation);//orientation = 1,0,0,0;
    (void)Q3Quaternion_SetRotate_Y(&orientation,45);//orientation = -0.8733, 0, -0.4871, 0;
    Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    Status3D = Q3Controller_SetButtons(DriverController,5);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    Status3D = Q3ControllerState_SaveAndReset (ControllerState);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);

    return 0;
}

static char * test_030(void)
{
    //preparation for the test of Q3ControllerState_Restore

    TQ3Uns32 lastTrackerProcCalled = trackerProcCalled;
    //override states:
    
    whatSet=0;
    Status3D = Q3Controller_SetChannel(ClientController,3,&whatSet,sizeof(whatSet));
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    //Mod tracker position
    position.x=1.;
    position.y=1.;
    position.z=1.;
    Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc wasn't called as trackerProcCalled unchanged", lastTrackerProcCalled != trackerProcCalled);
    
    lastTrackerProcCalled = trackerProcCalled;
    //Mod tracker orientation
    (void)Q3Quaternion_SetIdentity(&orientation);//orientation = 1,0,0,0;
    Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc wasn't called as trackerProcCalled unchanged", lastTrackerProcCalled != trackerProcCalled);
    
    lastTrackerProcCalled = trackerProcCalled;
    //Mod Buttons
    Status3D = Q3Controller_SetButtons(DriverController,7);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc wasn't called as trackerProcCalled unchanged", lastTrackerProcCalled != trackerProcCalled);
    
    Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 7);
    
    //Mod Values
    values[0]= 5.0;
    values[1]=-5.0;
    Status3D = Q3Controller_SetValues(DriverController,values,2);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong values", (values[0] == 5.0)&&(values[1] == -5.0) );
    mu_assert("error, not CtrlSerNum == 3", CtrlSerNum == 3);//CtrlSerNum = (QD3D = 2) 3;
    
    Status3D = Q3Controller_SetActivation(ClientController,kQ3False);//TODO: check if needed
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    
    //restore state
    Status3D = Q3ControllerState_Restore (ControllerState);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    
    //check if state changes between "save and reset" and "restore" are reverted
    //Check activation
    Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not controllerActivation == kQ3False", controllerActivation == kQ3False);//special case!
    
    //Check channel
    whatSize=sizeof(whatGet);
    Status3D = Q3Controller_GetChannel(ClientController,3,&whatGet,&whatSize);//to be fixed. Results in bad access if DeviceServer is not started!
#if 1
    //QD3D overides the returned status of Q3Controller_GetChannel with kQ3Success
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#else
    mu_assert("error, still kQ3Success", Status3D == kQ3Failure);
#endif
    mu_assert("error, not whatSize == 4", whatSize == 4);
    mu_assert("error, not whatGet == 1500", whatGet == 1500);
    
    //Check tracker position
    Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#if 1
    mu_assert("error, wrong position", (position.x == 1.)&&(position.y == 1.)&&(position.z == 1.) );//deactivated controller should have no effect on returned position
#else
    mu_assert("error, wrong position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
#endif
    
    //Check tracker orientation
    Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong orientation", (orientation.w == 1.)&&(orientation.x == 0.)&&(orientation.y == 0.)&&(orientation.z == 0.) );
    
    Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 7);
    
    Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong values", (values[0] == 5.0)&&(values[1] == -5.0) );
    mu_assert("error, not CtrlSerNum == 3", CtrlSerNum == 3);//TBC: QD3D - CtrlSerNum = 3;
    
    return 0;
};

static char * test_031(void)
{
    position.x=-1.;
    position.y=-1.;
    position.z=-1.;
    tempUns32=0;
    
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,10,&tempUns32,&position,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 5);
    mu_assert("error, wrong tacker position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,20,&tempUns32,&position,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 0);
    mu_assert("error, wrong tacker position", (position.x == 1.)&&(position.y == 1.)&&(position.z == 1.) );
    
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,30,&tempUns32,&position,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 0);
    mu_assert("error, wrong tacker position", (position.x == 2.)&&(position.y == 2.)&&(position.z == 2.) );
    
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,44,&tempUns32,&position,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 0);
    mu_assert("error, wrong tracker orientation", (orientation.w == 1.)&&(orientation.x == 0.)&&(orientation.y == 0.)&&(orientation.z == 0.) );
    
#if 1
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,50,(TQ3Uns32)NULL,NULL,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", FL_EQUAL(orientation.w, -0.8733047) && (orientation.x == 0.) && FL_EQUAL(orientation.y, -0.4871745) && (orientation.z == 0.) );
#else
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,50,&tempUns32,&position,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong buttons set", tempUns32 == 0);
    mu_assert("error, wrong tracker orientation", FL_EQUAL(orientation.w, -0.8733047) && (orientation.x == 0.) && FL_EQUAL(orientation.y, -0.4871745) && (orientation.z == 0.) );
    mu_assert("error, wrong tracker position", (position.x == 2.)&&(position.y == 2.)&&(position.z == 2.) );
#endif
    
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,10,&tempUns32,&position,&orientation);
    mu_assert("error, still event coordinates available", Status3D == kQ3Failure);
    
    return 0;
};

static char * test_032(void)
{
    //Set- and GetMethods; channels counting from 0 (zero)
    
    whatSet=150;
    Status3D = Q3Controller_SetChannel(ClientController, 0 ,&whatSet,sizeof(whatSet));
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    whatSet=250;
    Status3D = Q3Controller_SetChannel(ClientController, 1 ,&whatSet,sizeof(whatSet));
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    whatGet=0;
    whatSize=0;
    whatSize=sizeof(whatGet);
    Status3D = Q3Controller_GetChannel(ClientController, 0 ,&whatGet,&whatSize);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong return", whatGet == 250);
    mu_assert("error, wrong size", whatSize == 4);
    
    whatGet=0;
    whatSize=0;
    Status3D = Q3Controller_GetChannel(ClientController, 1 ,&whatGet,&whatSize);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong return", whatGet == 150);
    mu_assert("error, wrong size", whatSize == 4);
    
    whatGet=0;
    whatSize=0;
    Status3D = Q3Controller_GetChannel(ClientController, 2 ,&whatGet,&whatSize);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong return", whatGet == 4);
    mu_assert("error, wrong size", whatSize == 4);
    
    whatGet=0;
    whatSize=0;
    Status3D = Q3Controller_GetChannel(ClientController, 3 ,&whatGet,&whatSize);
#if 1
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#else
    mu_assert("error, still kQ3Success", Status3D == kQ3Failure);
#endif
    mu_assert("error, not whatGet == 1500", whatGet == 1500);
    mu_assert("error, wrong size", whatSize == 4);
    
    
    whatGet=0;
    whatSize=0;
    Status3D = Q3Controller_GetChannel(ClientController, 4 ,&whatGet,&whatSize);//
    //channel 4 is ambitous. remember: ChannelCount==4, channels 0 to 3!
#if 1
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not whatGet == 65000", whatGet == 65000); //from default case ChannelGetMethod
#else
    mu_assert("error, still kQ3Success", Status3D == kQ3Failure);
    mu_assert("error, not whatGet == 1500", whatGet == 1500); //TBC: same as channel 3 ?!
#endif
    mu_assert("error, wrong size", whatSize == 4);
    
    /* TBC:
     + observation:
     - shouldn't be successfull, but works!
     - should be kQ3Failure by ChannelGetMethod, but QD3D overides return value of ChannelGetMethod!
     - Method got called!! - QD3D-doc recommends 0<=channel<channelCount but defines no error!
     - TODO: (gracefull) pass; with whatSize=4; whatGet wasn't modified
     
     + Conclusion: QD3D ignores channelCount!
     
     + Solution: SetChannel and GetChannel are methods to communicate with the device driver.
     When the driver creates a new instance of a TQ3Controller via Q3Controller_New, it should keep
     a local copy of the TQ3ControllerData.ChannelCount value. Later the driver can check against this
     copy if the channel of SetChannel or GetChannel is out of Range and should behave gracefull.
     */
    
    return 0;
};

static char * test_033(void)
{
    Status3D = Q3Controller_SetActivation(ClientController,kQ3False);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Tracker_SetActivation(ClientTracker,kQ3False);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, controllerActivation ambitous", controllerActivation == kQ3False);
    
    return 0;
};

/*
 for follow up tests controller and tracker are now inactive!
 */

static char * test_034(void)
{
    //check Signature a 2nd time
    Status3D = Q3Controller_GetSignature(ClientController,tempString,5);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong string returned", 0==strcmp("com.", tempString));
    
    Status3D = Q3Controller_GetSignature(ClientController,tempString,128);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong string returned", 0==strcmp("com.HMDP.BenchController", tempString));
    
    return 0;
};

static char * test_035(void)
{
    //Note: controller and tracker are both inactive!
    
    Status3D = Q3Controller_GetValueCount(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempUns32 == 16", tempUns32 == 16);
    
    Status3D = Q3Controller_Track2DCursor(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3False", tempBool == kQ3False);
    
    Status3D = Q3Controller_Track3DCursor(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not tempBool == kQ3False", tempBool == kQ3False);
    
    return 0;
};

static char * test_036(void)
{
    //Note: controller and tracker are both inactive!
    
    TQ3Uns32 lastTrackerProcCalled = trackerProcCalled;
    Status3D = Q3Controller_SetButtons(DriverController,5);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc was called but Controller is inactive", lastTrackerProcCalled == trackerProcCalled);
    
    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#if 1
    mu_assert("error, ambitous, wrong buttons set", tempUns32 == 0);
#else
    mu_assert("error, ambitous, wrong buttons set", tempUns32 == 7);
#endif
    
    tempUns32 = 0;
    Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, ambitous, wrong buttons set", tempUns32 == 7);
    
    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, buttons not cleared. Buttons were returned the call before, then the tracker set buttons bitfield to zero.", tempUns32 == 0);
   
    return 0;
};

static char * test_037(void)
{
    TQ3Uns32 lastTrackerProcCalled;
    //Note: controller and tracker are both inactive!
    
    TrackerSerNum = 0;
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,NULL,NULL,&TrackerSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);// Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated
    
    lastTrackerProcCalled = trackerProcCalled;
    position.x=1.;
    position.y=1.;
    position.z=1.;
    Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, trackerProc was called but Controller is inactive", lastTrackerProcCalled == trackerProcCalled);
    
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    mu_assert("error, wrong tracker deltaPos", (deltaPos.x == 0.)&&(deltaPos.y == 0.)&&(deltaPos.z == 0.) );
    mu_assert("error, tracker changed", changed == kQ3False);
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);// Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated
    
    Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    
    return 0;
};

static char * test_038(void)
{
    //Note: controller and tracker are both inactive!
    
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,NULL,NULL,&TrackerSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);// Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated
    
    //mu_run_test(test_004);
    deltaPos.x=1.;
    deltaPos.y=1.;
    deltaPos.z=1.;
    Status3D = Q3Controller_MoveTrackerPosition(DriverController,&deltaPos);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    mu_assert("error, wrong tracker deltaPos", (deltaPos.x == 0.)&&(deltaPos.y == 0.)&&(deltaPos.z == 0.) );
    mu_assert("error, not changed == kQ3False", changed == kQ3False);
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);// Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated
    
    return 0;
};

static char * test_039(void)
{
    //Note: controller and tracker are both inactive!
    
    Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,NULL,NULL,&TrackerSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );//orientation = 1,-0,-0,-0
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);
    
    (void)Q3Quaternion_SetIdentity(&orientation);
    Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );//orientation = 1,-0,-0,-0
    
    Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);
	mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );
    mu_assert("error, wrong tracker deltaOri", (deltaOri.w == 1.)&&(deltaOri.x == 0)&&(deltaOri.y == 0)&&(deltaOri.z == 0) );//deltaOri = 1,-0,-0,-0;
    mu_assert("error, not changed == kQ3False", changed == kQ3False);
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);
    
    (void)Q3Quaternion_SetRotate_Y(&deltaOri,45); //deltaOri = -0.8733047,0,-0.4871745,0;
    Status3D = Q3Controller_MoveTrackerOrientation(DriverController,&deltaOri);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );
    
    Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);
	mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong tracker orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );
    mu_assert("error, wrong tracker deltaOri", (deltaOri.w == 1.)&&(deltaOri.x == 0)&&(deltaOri.y == 0)&&(deltaOri.z == 0) );//deltaOri = 1,-0,-0,-0;
    mu_assert("error, not changed == kQ3False", changed == kQ3False);
    mu_assert("error, not TrackerSerNum == 0", TrackerSerNum == 0);
    
    return 0;
};

static char * test_040(void)
{
    Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong values", (values[0] == 5.)&&(values[1] == -5.) );
    mu_assert("error, not CtrlSerNum == 3", CtrlSerNum == 3);
    
    values[0]= 3.0;
    values[1]=-3.0;
    Status3D = Q3Controller_SetValues(DriverController,values,2);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    changed = kQ3True;
    Status3D = Q3Controller_GetValues(ClientController,2,values,&changed,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, values overwritten", (values[0] == 3.)&&(values[1] == -3.) );
    mu_assert("error, flag 'changed' overwritten", changed == kQ3False);
#if 1
    mu_assert("error, not CtrlSerNum == 3; ambitous", CtrlSerNum == 3);
#else
    mu_assert("error, not CtrlSerNum == 4; ambitous", CtrlSerNum == 4);
#endif    
    return 0;
};

static char * test_041(void)
{
    //controller decommisioned

    Status3D = Q3Controller_Decommission(DriverController); //TODO: what happens to assigned tracker?
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    
    Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, not controllerActivation == kQ3False", controllerActivation == kQ3False);
    
    Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, has still tracker", tempBool == kQ3False);
    
    Status3D = Q3Controller_GetSignature(ClientController,tempString,5);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#if 1
    mu_assert("error, string not empty", 0==strlen(tempString));
#else
    mu_assert("error / ambitous: wrong string returned", 0==strcmp("com.", tempString));
    //ambitous pass: Quesa returns signature even when the contoller is decommissioned: "resonable default value"
#endif
    
    Status3D = Q3Controller_GetSignature(ClientController,tempString,128);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
#if 1
    mu_assert("error, string not empty", 0==strlen(tempString));
#else
    mu_assert("error / ambitous: wrong string returned", 0==strcmp("com.HMDP.BenchController", tempString));
    //ambitous pass: Quesa returns signature even when the contoller is decommissioned: "resonable default value"
#endif
    
    Status3D = Q3Controller_GetValueCount(ClientController,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong valuecount", tempUns32 == 0);
    
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, buttons set", tempUns32 == 0);
    
    Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong position", (position.x == 0.)&&(position.y == 0.)&&(position.z == 0.) );
    
    Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, wrong orientation", (orientation.w == 1.)&&(orientation.x == 0)&&(orientation.y == 0)&&(orientation.z == 0) );//orientation = 1,0,0,0
    
    CtrlSerNum=0;
    values[0]=+5;
    values[1]=-5;
    Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, values overwritten", (values[0] == +5)&&(values[1] == -5) );
#if 1
    mu_assert("error, not CtrlSerNum == 0", CtrlSerNum == 0);
#else
    mu_assert("error, not CtrlSerNum == 4", CtrlSerNum == 4);
#endif
    
    return 0;
};

static char * test_042(void)
{
    DriverController = Q3Controller_New(&ControllerData);//controller re-new
    mu_assert("error, DriverController == NULL", DriverController != NULL);
    
    Status3D = Q3Controller_Next(NULL,&tempController);
    mu_assert("error, no kQ3Success", Status3D == kQ3Success);
    mu_assert("error, any other controller available", tempController != NULL);
    
    return 0;
};

static char * test_043(void)
{
    //dispose tracker
    Q3Object_Dispose(ClientTracker);
    //done: check/review of Tracker Dispose
    
    Q3Object_Dispose(ControllerState);
    //done: check/review of OS X Quesa cleanup
    
    return 0;
};

char * do_bench(void)
{
#ifndef BenchX
    printf("Hello World, this is the CodeWarrior console!\n");
#endif
    //init of globals
    controllerListSerialNumber = 0;
    trackerProcCalled = 0;
    doBeep = 0;
    tempController = NULL;
    
    mu_run_test(test_000);
    mu_run_test(test_001);
    mu_run_test(test_002);
    mu_run_test(test_003);
    mu_run_test(test_004);
    mu_run_test(test_005);
    mu_run_test(test_006);
    mu_run_test(test_007);
    mu_run_test(test_008);
    mu_run_test(test_009);
    
    mu_run_test(test_010);
    mu_run_test(test_011);
    mu_run_test(test_012);
    mu_run_test(test_013);
    mu_run_test(test_014);
    mu_run_test(test_015);
    mu_run_test(test_016);
    mu_run_test(test_017);
    mu_run_test(test_018);
    mu_run_test(test_019);
    
    mu_run_test(test_020);
    mu_run_test(test_021);
    mu_run_test(test_022);
    mu_run_test(test_023);
    mu_run_test(test_024);
    mu_run_test(test_025);
    mu_run_test(test_026);
    mu_run_test(test_027);
    mu_run_test(test_028);
    mu_run_test(test_029);
    
    mu_run_test(test_030);
    mu_run_test(test_031);
    mu_run_test(test_032);
    mu_run_test(test_033);
    mu_run_test(test_034);
    mu_run_test(test_035);
    mu_run_test(test_036);
    mu_run_test(test_037);
    mu_run_test(test_038);
    mu_run_test(test_039);
    
    mu_run_test(test_040);
    mu_run_test(test_041);
    mu_run_test(test_042);
    
    mu_run_test(test_006);
    mu_run_test(test_018);
    mu_run_test(test_019);
    
    mu_run_test(test_043);
 
#if 1
    debug("PASS. Tests run: %d",tests_run);
#endif
    return 0;
}
