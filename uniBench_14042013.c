/*
 *  uniBench.c
 *  QuesaControllerBenchX
 *
 *  Created by Ole Hartmann on Wed Oct 08 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef BenchX
#define QUESA_OS_MACINTOSH 1
#define Q3_DEBUG 1

#include <Quesa/Quesa.h>
#include <Quesa/QuesaController.h>
#include <Quesa/QuesaMath.h>
#else
#include <QD3D.h>
#include <QD3DController.h>
#include <QD3DMath.h>
#include <Sound.h>
#endif
#include "uniBench.h"

TQ3Uns32	lastsize, what0, what1, what_work, trackerProcCalled;


//callback functions

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
				what0=NULL;
			else
				what0=*(TQ3Uns32*)data;
			break;
		case 1:
			if (data==NULL)
				what1=NULL;
			else
				what1=*(TQ3Uns32*)data;
			break;
		case 3:
			if (data==NULL)
				what_work=NULL;
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
#ifdef BenchX
    NSBeep();
#else
	SysBeep(100);
#endif
    trackerProcCalled++;
    return kQ3Success;
};

void do_bench(void)
{
	TQ3Boolean			controllerListChanged;
    TQ3Uns32			controllerListSerialNumber = 0;
    TQ3ControllerData	ControllerData;
	TQ3ControllerStateObject
    ControllerState;
	TQ3Status			Status3D;
	TQ3ControllerRef	tempController = NULL;
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
    
    trackerProcCalled = 0;
    
	Status3D = Q3Initialize();//take a look at registration of TQ3TrackerObject!!
	/*
     Status3D = kQ3Success
     */
    //passed
	
	Status3D = Q3Controller_Next(NULL,&ClientController);
	/*
     Status3D = kQ3Success
     ClientController = NULL
     */
    //passed
	
	Status3D = Q3Controller_GetListChanged(&controllerListChanged,&controllerListSerialNumber);
	/*
     QD3D
     Status3D = kQ3Success
     controllerListChanged = kQ3True
     controllerListSerialNumber = 1; at first start
     */
	/*
     Quesa
     Status3D = kQ3Success
     controllerListChanged = kQ3False
     controllerListSerialNumber = 0; at first start
     */
    //passed
#warning Q3Controller_GetListChanged: Check against QD3D doc
	
	Status3D = Q3Controller_Next(tempController,&ClientController);
	/*
     Status3D = kQ3Success;
     ClientController = NULL!! //should be NULL in OSX too
     */
    //passed
	
	//Fill struct TQ3ControllerData for Controller Creation
	
	ControllerData.signature="com.HMDP.BenchController\0";
	ControllerData.valueCount=16;//max valueCount is unknown for QD3D!! seems to be unlimited (4096 worked)
	
	ControllerData.channelCount=4;
	ControllerData.channelGetMethod=ChannelGetMethod;
	ControllerData.channelSetMethod=ChannelSetMethod;
	
	//ControllerData.channelCount=0;
	//ControllerData.channelGetMethod=NULL;
	//ControllerData.channelSetMethod=NULL;
	
	DriverController = Q3Controller_New(&ControllerData);
	/*
     DriverController != NULL
     */
    //passed
	
	Status3D = Q3Controller_GetListChanged(&controllerListChanged,&controllerListSerialNumber);
	/*
     Status3D = kQ3Success
     controllerListChanged = kQ3True
     controllerListSerialNumber = (QD3D: 2; Quesa: 1)
     */
    //passed
    
	Status3D = Q3Controller_Next(NULL,&ClientController);
	/*
     Status3D = kQ3Success
     ClientController = DriverController!!
     */
    //passed
    
	ControllerState = Q3ControllerState_New (ClientController);
	/*
     ControllerState != NULL
     */
    //passed
	
	Status3D = Q3Controller_Next(ClientController,&tempController);
	/*
     Status3D = kQ3Success;
     QD3D: tempController != NULL;
     Quesa: Should be NULL in OSX - no System Cursor?!
     */
    //passed for Quesa: NULL returned
    
	//New Tracker
	ClientTracker = Q3Tracker_New(TrackerProc);
	/*
     ClientTracker = Q3Tracker_New(NULL);
     ClientTracker != NULL
     */
    //passed
	
	Status3D = Q3Tracker_GetNotifyThresholds(ClientTracker,&posTresh,&oriTresh);
	/*
     Status3D = kQ3Success;
     posTresh = 0.0;
     oriTresh = 0.0;
     */
    //passed
	
	posTresh = 0.1;
	oriTresh = 0.1;
	Status3D = Q3Tracker_SetNotifyThresholds(ClientTracker,posTresh,oriTresh);
	/*
     Status3D = kQ3Success
     */
    //passed
	
	Status3D = Q3Tracker_GetNotifyThresholds(ClientTracker,&posTresh,&oriTresh);
	/*
     Status3D = kQ3Success;
     posTresh = 0.1;
     oriTresh = 0.1;
     */
    //passed
    
	posTresh = 0.0;
	oriTresh = 0.0;
	Status3D = Q3Tracker_SetNotifyThresholds(ClientTracker,posTresh,oriTresh);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	//deactivate Tracker
	Status3D = Q3Tracker_SetActivation(ClientTracker,kQ3False);
	/*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D =  Q3Tracker_GetActivation(ClientTracker,&trackerActivation);
	/*
     Status3D = kQ3Success;
     trackerActivation = kQ3False;
     */
    //passed
	
	Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
	//indicates a active controller !and! a active tracker
	/*
     Status3D = kQ3Success;
     tempBool = kQ3True; should be kQ3False; !TBC!
     */
	//Quesa: tempBool = kQFalse; no Tracker associated!
    //passed for Quesa
	
	Status3D = Q3Controller_SetTracker(ClientController,ClientTracker);
	//Status3D = kQ3Success;
    //passed
	
	Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
	/*
     Status3D = kQ3Success;
     controllerActivation = kQ3True;
     */
    //passed
	
	//inactive Tracker
	Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3False;
     */
    //passed
	
	//active Tracker
	Status3D = Q3Tracker_SetActivation(ClientTracker,kQ3True);
	/*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D =  Q3Tracker_GetActivation(ClientTracker,&trackerActivation);
	/*
     Status3D = kQ3Success;
     trackerActivation = kQ3True;
     */
    //passed
    
	Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3True;
     */
    //passed
	
	//client actions
	Status3D = Q3Controller_GetSignature(ClientController,tempString,5);
	/*
     Status3D = kQ3Success;
     tempString = "com.";
     */
    //passed
    
	Status3D = Q3Controller_GetSignature(ClientController,tempString,128);
	/*
     Status3D = kQ3Success;
     tempString = "com.HMDP.BenchController";
     */
    //passed
    
	Status3D = Q3Controller_GetValueCount(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 16;
     */
    //passed
	
	//driver actions - client query (DriverController)
	Status3D = Q3Controller_Track2DCursor(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3False;
     */
    //passed
    
	Status3D = Q3Controller_Track3DCursor(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3False;
     */
    //passed
	
	Status3D = Q3Controller_SetButtons(DriverController,5);
	//Take a look at Q3Tracker_ChangeButtons
	//Status3D = kQ3Success;
	//TrackerProc gets called -> breakpoint above should be hit
    //passed; TODO: check break on TrackerProc
    
	tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 5;
     */
    //passed
    
	tempUns32 = 0;
    Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 5; could be 0
     */
    //passed; returned 5
    
	tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 5; could be 0
     */
    //passed: was 0! this is intentional! Buttons were returned the call before, then the tracker set buttons bitfield to zero.
	
	TrackerSerNum=0;
	Status3D = Q3Tracker_GetPosition(ClientTracker,&position,NULL,NULL,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     TrackerSerNum = 1;
     */
    //passed
	
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,10,NULL,NULL,NULL);
	/*
     Status3D = kQ3Failure;
     */
    //passed
	
	tempUns32=5;
	Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,10,tempUns32,&position,NULL);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	position.x=1.;
	position.y=1.;
	position.z=1.;
	Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
	/*
     Status3D = kQ3Success;
     TrackerProc gets called -> breakpoint above should be hit
     */
    //passed
    
	Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     position = 1,1,1;
     deltaPos = 0,0,0;
     changed = kQ3True;
     TrackerSerNum = 2;
     */
    //passed
	
	Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,20,NULL,&position,NULL);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
	/*
     Status3D = kQ3Success;
     position = 1,1,1;
     */
    //passed
    
	Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     position = 1,1,1;
     deltaPos = 0,0,0;
     changed = kQ3False;
     TrackerSerNum = 2;
     */
    //passed
    
	deltaPos.x=1.;
	deltaPos.y=1.;
	deltaPos.z=1.;
	Status3D = Q3Controller_MoveTrackerPosition(DriverController,&deltaPos);
	/*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
	/*
     Status3D = kQ3Success;
     position = 2,2,2;
     */
    //passed
    
	Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     position = 2,2,2;
     deltaPos = 0,0,0;//why no delta? Doc? Threshold? previously cleared?
     changed = kQ3True;
     TrackerSerNum = 3;
     */
    //passed; deltaPos is {0,0,0}
	
	Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,30,NULL,&position,NULL);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	TrackerSerNum=0;
	Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,NULL,NULL,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     orientation = 1,-0,-0,-0;//w,x,y,z
     serNum = 1;
     */
    //passed
	
	Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,40,NULL,NULL,&orientation);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	(void)Q3Quaternion_SetIdentity(&orientation);
	//orientation = 1,0,0,0;
	Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
	/*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
	/*
     Status3D = kQ3Success;
     orientation = 1,-0,-0,-0;//w,x,y,z
     */
    //passed
    
	Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     orientation = 1,0,0,0;
     deltaOri = 1,-0,-0,-0;
     changed = kQ3True;
     TrackerSerNum = 2;
     */
    //passed
    
	(void)Q3Quaternion_SetRotate_Y(&deltaOri,45);
	//deltaOri = -0.8733047,0,-0.4871745,0;
	Status3D = Q3Controller_MoveTrackerOrientation(DriverController,&deltaOri);
	/*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
	/*
     Status3D = kQ3Success;
     orientation = -0.8733047,-0,-0.4871745,-0;
     */
    //passed
    
	Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     orientation = -0.8733047,-0,-0.4871745,-0;
     deltaOri = 1,0,0,0;
     changed = kQ3True;
     TrackerSerNum = 3;
     */
    //passed
	
	Status3D = Q3Tracker_SetEventCoordinates(ClientTracker,50,NULL,NULL,&orientation);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
	/*
     Status3D = kQ3Success;
     values = 0.0,0.0;
     CtrlSerNum = 1;
     */
    //passed
    
	values[0]= 1.0;
	values[1]=-1.0;
	Status3D = Q3Controller_SetValues(DriverController,values,2);
	/*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D = Q3Controller_GetValues(ClientController,2,values,&changed,&CtrlSerNum);
	/*
     Status3D = kQ3Success;
     values = 1.0,-1.0;
     changed = kQ3True;
     CtrlSerNum = 2;
     */
    //passed
	
	//Mod channel
	whatSet=1500;
	Status3D = Q3Controller_SetChannel(ClientController,3,&whatSet,sizeof(whatSet));
    /*
     Status3D = kQ3Success;
     */
    //passed
    
	//Mod tracker position
	position.x=0.;
	position.y=0.;
	position.z=0.;
	Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
	//Mod tracker orientation
    /*
     Status3D = kQ3Success;
     */
    //passed
    
	(void)Q3Quaternion_SetIdentity(&orientation);
    //orientation = 1,0,0,0;
	(void)Q3Quaternion_SetRotate_Y(&orientation,45);
    //orientation = -0.8733, 0, -0.4871, 0;
	Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
    /*
     Status3D = kQ3Success;
     */
    //passed
    
	//Mod Buttons
	Status3D = Q3Controller_SetButtons(DriverController,5);
    /*
     Status3D = kQ3Success;
     */
    //passed
    
	Status3D = Q3ControllerState_SaveAndReset (ControllerState);
	/*
     Status3D = kQ3Success
     */
    //passed
	
	//Mod channel
	whatSet=0;
	Status3D = Q3Controller_SetChannel(ClientController,3,&whatSet,sizeof(whatSet));
    /*
     Status3D = kQ3Success
     */
    //passed
    
	//Mod tracker position
	position.x=1.;
	position.y=1.;
	position.z=1.;
	Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
    /*
     Status3D = kQ3Success
     SysBeep
     */
    //passed
    
	//Mod tracker orientation
	(void)Q3Quaternion_SetIdentity(&orientation);
	//orientation = 1,0,0,0;
	Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
    /*
     Status3D = kQ3Success
     SysBeep
     */
    //passed
    
	//Mod Buttons
	Status3D = Q3Controller_SetButtons(DriverController,7);
    /*
     Status3D = kQ3Success
     SysBeep
     */
    //passed
    
	Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 7;
     */
    //passed
	
	//Mod Values
	values[0]= 5.0;
	values[1]=-5.0;
	Status3D = Q3Controller_SetValues(DriverController,values,2);
    /*
     Status3D = kQ3Success
     */
    //passed
    
	Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
	/*
     Status3D = kQ3Success;
     values = 5.0,-5.0;
     CtrlSerNum = (QD3D = 2) 3;
     */
    //passed; returned 3
    
	//Mod activation
	//Status3D = Q3Controller_SetActivation(ClientController,kQ3False);
	
	Status3D = Q3ControllerState_Restore (ControllerState);
	/*
     Status3D = kQ3Success;
     */
    //passed
	
	//Check activation
	Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
	/*
     controllerActivation = kQ3False;
     Status3D = kQ3Success;
     */
    //FAIL: controllerActivation!=kQ3False
	
	//Check channel
	whatSize=sizeof(whatGet);
	Status3D = Q3Controller_GetChannel(ClientController,3,&whatGet,&whatSize);
	/*
     whatGet = 1500;
     whatSize = 4;
     Status3D = kQ3Success;
     */
    //passed
	
	//Check tracker position
	Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
	/*
     position = 1,1,1;
     Status3D = kQ3Success;
     */
    //passed
	
	//Check tracker orientation
	Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
	/*
     orientation = 1, 0, 0, 0;
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 7; (could be 0)
     */
    //passed; was 7
	
	Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
	/*
     Status3D = kQ3Success;
     values = 5.0,-5.0;
     CtrlSerNum = 3;
     */
	//passed
    
    position.x=-1.;
	position.y=-1.;
	position.z=-1.;
    tempUns32=0;
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,10,&tempUns32,&position,&orientation);
	/*
     tempUns32 = 5;
     position = {0,0,0};
     orientation = no value;
     Status3D = kQ3Success;
     */
    //passed; position 0,0,0
	
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,20,&tempUns32,&position,&orientation);
	/*
     tempUns32 = 0;
     position = {1,1,1};
     orientation = no value;
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,30,&tempUns32,&position,&orientation);
	/*
     tempUns32 = 0;
     position = {2,2,2};
     orientation = no value;
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,44,&tempUns32,&position,&orientation);
	/*
     tempUns32 = 0;
     position =  no value;
     orientation = {1,0,0,0};
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,50,&tempUns32,&position,&orientation);
	/*
     tempUns32 = 0;
     position = no value;
     orientation = {-0.87330,0,-0.48717,0};
     Status3D = kQ3Success;
     */
    //passed
	
	Status3D = Q3Tracker_GetEventCoordinates(ClientTracker,10,&tempUns32,&position,&orientation);
	/*
     tempUns32 = ;
     position = ;
     orientation = ;
     Status3D = kQ3Failure;
     */
    //passed
	
	//Set- and GetMethods
	//channels counting from 0 (zero)
	whatSet=150;
	Status3D = Q3Controller_SetChannel(ClientController,0,&whatSet,sizeof(whatSet));
	/*
     Status3D = kQ3Success
     */
    //passed
	
	whatSet=250;
	Status3D = Q3Controller_SetChannel(ClientController,1,&whatSet,sizeof(whatSet));
	/*
     Status3D = kQ3Success
     */
    //passed
	
	Status3D = Q3Controller_SetChannel(ClientController,3,&whatSet,sizeof(whatSet));
	/*
     Status3D = kQ3Success;
     remember: ChannelCount=3
     shouldn't be successfull, but works - QD3D-doc recommends 0<=channel<channelCount but defines no error!
     */
    //passed with kQ3Success
	
	whatSize=sizeof(whatGet);
	Status3D = Q3Controller_GetChannel(ClientController,0,&whatGet,&whatSize);
	/*
     Status3D = kQ3Success;
     whatGet = 250; should be 250
     whatSize = 4; should be 4
     */
    //passed
	
	Status3D = Q3Controller_GetChannel(ClientController,1,&whatGet,&whatSize);
    /*
     Status3D = kQ3Success;
     whatGet = 150; should be 150
     whatSize = 4; should be 4
     */
    //passed
	
	Status3D = Q3Controller_GetChannel(ClientController,2,&whatGet,&whatSize);
    /*
     Status3D = kQ3Success;
     whatGet = 4; should be 4
     whatSize = 4; should be 4
     */
	//passed
    
	whatGet=0;
	whatSize=0;
	Status3D = Q3Controller_GetChannel(ClientController,3,&whatGet,&whatSize);
	//Status3D = kQ3Success;
	//ChannelCount=3
	//shouldn't be successfull, but works
	//should be kQ3Failure by ChannelGetMethod, but QD3D overides return value of ChannelGetMethod!
	//Method got called!! - QD3D-doc recommends 0<=channel<channelCount but defines no error!
	//whatGet = 65000; returned by ChannelGetMethod
	//whatSize = 4; returned by ChannelGetMethod
	
	/*-Conclusion: QD3D ignores channelCount
     -solution: SetChannel and GetChannel are methods to communicate with the device driver.
     When the driver creates a new instance of a TQ3Controller via Q3Controller_New, it should keep
     a local copy of the TQ3ControllerData.ChannelCount value. Later it can check against this
     copy, if the channel of SetChannel or GetChannel is out of Range.
     */
    //(gracefull) pass; with whatsize=4; whatget wasn't modified
	
	//!controller inactive!
	Status3D = Q3Controller_SetActivation(ClientController,kQ3False);
    /*
     Status3D = kQ3Success
     */
    //passed
	
	//tracker inactive
	//deactivate Tracker
	Status3D = Q3Tracker_SetActivation(ClientTracker,kQ3False);
    /*
     Status3D = kQ3Success
     */
    //passed
	
	Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
	/*
     Status3D = kQ3Success;
     controllerActivation = kQ3True;//ambitous: shall be kQ3False as the controller was deactivated before
     */
    //ambitous pass
	
	//client actions
	Status3D = Q3Controller_GetSignature(ClientController,tempString,5);
	/*
     Status3D = kQ3Success;
     tempString = "com.";
     */
    //passed
    
	Status3D = Q3Controller_GetSignature(ClientController,tempString,128);
	/*
     Status3D = kQ3Success;
     tempString = "com.HMDP.BenchController";
     */
    //passed
    
	Status3D = Q3Controller_GetValueCount(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 16;
     */
    //passed
	
	//driver actions - client query (DriverController)
	Status3D = Q3Controller_Track2DCursor(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3False;
     */
    //passed
    
	Status3D = Q3Controller_Track3DCursor(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3False;
     */
    //passed
	
	Status3D = Q3Controller_SetButtons(DriverController,5);
	/* Take a lock at Q3Tracker_ChangeButtons
     Status3D = kQ3Success;
     TrackerProc gets called
     */
    //passed; TODO: check TrackerProc
    
	tempUns32=0;
    Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 0;//ambitious! Tracker is inactive; when active last value of Buttons was 7
     */
    //ambitous pass; returned 7
    
	Status3D = Q3Controller_GetButtons(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 5;//ambitious! Controller is inactive, so SetButtons should have no effect; when active last value of Buttons was 7 
     */
    //ambitous pass; returned 7
    
	Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 0;
     */
    //passed
	
	TrackerSerNum = 0;
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,NULL,NULL,&TrackerSerNum);//TBC
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     serNum = 0;
     */
    //passed; Q3Tracker_GetPosition does not overwrite TrackerSerNum when deactivated
    
	position.x=1.;
	position.y=1.;
	position.z=1.;
	Status3D = Q3Controller_SetTrackerPosition(DriverController,&position);
	/*
     Status3D = kQ3Success;
	 TrackerProc gets called
     */
    //passed
	
    Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     deltaPos = 0,0,0;
     changed = kQ3False;
     TrackerSerNum = 3; No Overwriting!!
     */
    //passed
    
	Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     */
    //passed
    
	Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     deltaPos = 0,0,0;
     changed = kQ3False;
     TrackerSerNum = 3;//No Overwriting!!
     */
    //passed
    
	deltaPos.x=1.;
	deltaPos.y=1.;
	deltaPos.z=1.;
	Status3D = Q3Controller_MoveTrackerPosition(DriverController,&deltaPos);
	//Status3D = kQ3Success;
	Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     */
    //passed
    
    
	Status3D = Q3Tracker_GetPosition(ClientTracker,&position,&deltaPos,&changed,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     position = 0,0,0;
     deltaPos = 0,0,0;
     changed = kQ3False;
     TrackerSerNum = 3;//No Overwriting!!
     */
    //passed
	
	Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,NULL,NULL,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     orientation = 1,0,0,0;//w,x,y,z
     serNum = 0;
     */
    //passed
    
	(void)Q3Quaternion_SetIdentity(&orientation);
	//orientation = 1,0,0,0;
	Status3D = Q3Controller_SetTrackerOrientation(DriverController,&orientation);
	//Status3D = kQ3Success;
	Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
	/*
     Status3D = kQ3Success;
     orientation = 1,0,0,0;//w,x,y,z
     */
    //passed
    
	Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     orientation = 1,0,0,0;
     deltaOri = 1,0,0,0;
     changed = kQ3False;
     TrackerSerNum = 3;//No Overwriting!!
     */
    //passed
    
	(void)Q3Quaternion_SetRotate_Y(&deltaOri,45);
	//deltaOri = -0.8733047,0,-0.4871745,0;
	Status3D = Q3Controller_MoveTrackerOrientation(DriverController,&deltaOri);
	//Status3D = kQ3Success;
	Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);//TBC
	/*
     Status3D = kQ3Success;
     orientation = -0.8733047,-0,-0.4871745,-0;
     */
    //passed
    
	Status3D = Q3Tracker_GetOrientation(ClientTracker,&orientation,&deltaOri,&changed,&TrackerSerNum);
	/*
     Status3D = kQ3Success;
     orientation = 1,0,0,0;
     deltaOri = 1,0,0,0;
     changed = kQ3False;
     TrackerSerNum = 3;//No Overwriting!!
     */
    //passed
	
	Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
	/*
     Status3D = kQ3Success;
     values = 5.0,-5.0;
     CtrlSerNum = 3;
     */
    //passed
    
	values[0]= 3.0;
	values[1]=-3.0;
	Status3D = Q3Controller_SetValues(DriverController,values,2);
	/*
     Status3D = kQ3Success
     */
    //passed
    
	values[0]= 0.;
	values[1]= 0.;
    Status3D = Q3Controller_GetValues(ClientController,2,values,&changed,&CtrlSerNum);//TBC
	/*
     Status3D = kQ3Success;
     values = 3.0,-3.0;
     changed = kQ3True;//ambitous, as the controller is inactive; should be kQ3False
     CtrlSerNum = 4;   //ambitous QD3D documentation! Even when the controller is inactive Q3Controller_SetValues has an effect!
     */
    //ambitous pass with CtrlSerNum == 4
	
	//controller decommisioned
	Status3D = Q3Controller_Decommission(DriverController); //what happens to assigned tracker?
	/*
     Status3D = kQ3Success
     */
    //passed
    
	//client actions
	Status3D = Q3Controller_GetActivation(ClientController,&controllerActivation);
	/*
     Status3D = kQ3Success;
     controllerActivation = kQ3False;
     */
    //passed
    
	Status3D = Q3Controller_HasTracker(DriverController,&tempBool);
	/*
     Status3D = kQ3Success;
     tempBool = kQ3False;
     */
    //passed
	
	//client query (DriverController)
	Status3D = Q3Controller_GetSignature(ClientController,tempString,5);
	/*
     Status3D = kQ3Success;
     tempString = "";// null-string
     */
    //ambitous pass: Quesa returns signature even when the contoller is decommissioned: "resonable default value"
    
	Status3D = Q3Controller_GetSignature(ClientController,tempString,128);
	/*
     Status3D = kQ3Success;
     tempString = "";// null-string
     */
    //ambitous pass: Quesa returns signature even when the contoller is decommissioned: "resonable default value"
    
	Status3D = Q3Controller_GetValueCount(ClientController,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 0;
     */
    //passed
	
	Status3D = Q3Tracker_GetButtons(ClientTracker,&tempUns32);
	/*
     Status3D = kQ3Success;
     tempUns32 = 5; should be 5; may be undefined
     */
    //passed: returned 0
	
	Status3D = Q3Controller_GetTrackerPosition(ClientController,&position);
	/*
     Status3D = kQ3Success;
     position = 0.0,0.0,0.0;
     */
    //passed
    
	Status3D = Q3Controller_GetTrackerOrientation(ClientController,&orientation);
	/*
     Status3D = kQ3Success;
     orientation = 1,0,0,0;//w,x,y,z
     */
    //passed
	
	CtrlSerNum=0;
	values[0]=.0;
	values[1]=.0;
	Status3D = Q3Controller_GetValues(ClientController,2,values,NULL,&CtrlSerNum);
	/*
     Status3D = kQ3Success;
     values = 0.0,0.0;//no modification
     CtrlSerNum = 5;//no overwriting!!
     */
    //ambitous pass; CtrlSerNum returned 4
	
	//controller re-new
	DriverController = Q3Controller_New(&ControllerData); //step into!!
	//passed!
    
	//dispose tracker
	Q3Object_Dispose(ClientTracker);
#warning Tracker Dispose not done!
	
	Q3Object_Dispose(ControllerState);
	
#warning OS X Quesa cleanup not done!
    
}

