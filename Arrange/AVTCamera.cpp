//
//  AVTCamera.cpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/07.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifdef USE_AVT_CAM

#include "AVTCamera.h"
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include "XGetopt.h"
#endif

#ifndef _WINDOWS
#define strncpy_s(dest,len,src,count) strncpy(dest,src,count)
#define sprintf_s(dest,len,format,args...) sprintf(dest,format,args)
#define sscanf_s sscanf
#define strcpy_s(dst,len,src) strcpy(dst,src)
#define _strdup(src) strdup(src)
#define strtok_s(tok,del,ctx) strtok(tok,del)
#endif

#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <strings.h>
#endif

#ifdef _WINDOWS
#define _STDCALL __stdcall
#else
#define _STDCALL
#define TRUE     0
#endif

using namespace std;
using namespace MLARR::IO;
using namespace MLARR::Basic;

/*-------------------------------*/
/* Static members & functions    */
/*-------------------------------*/

std::vector<AVTCamera*> AVTCamera::vecCam;

#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
//Define function equivalent to Windows Sleep
void Sleep(unsigned int time)
{
    struct timespec t,r;
    
    t.tv_sec    = time / 1000;
    t.tv_nsec   = (time % 1000) * 1000000;
    
    while(nanosleep(&t,&r)==-1)
        t = r;
}

//Define function equivalent to Windows SetConsoleCtrlHandler
void SetConsoleCtrlHandler(void (*func)(int), int junk)
{
    signal(SIGINT, func);
}

#endif

// CTRL+C handler
#ifdef _WINDOWS
BOOL WINAPI AVTCamera::CtrlCHandler(DWORD dwCtrlType)
#else
void CtrlCHandler(int Signo)
#endif
{
    for( std::vector<AVTCamera*>::iterator it = AVTCamera::vecCam.begin(); it != AVTCamera::vecCam.end(); it++ ){
        
        cout << "Ctrl+C interrupt received, stopping camera : " << *it;
    
        (*it)->Abort = true;

        // exit(-1);
    }
    
#ifndef _WINDOWS
    signal(SIGINT, CtrlCHandler);
#else
    return true;
#endif
}

void convertRGB24toGray( unsigned char* src, unsigned char* dst, const int width, const int height ){

    for( int i = 0; i < height; i++){
        for( int j = 0; j < width; j++){
            int sum = *(src + 3 * (width*i + j)) + *(src + 3 * (width*i + j)+1) + *(src + 3 * (width*i + j) +2);
            *( dst + width*i + j ) = static_cast<unsigned char>( sum / 3 );
        }
    }

}

void _STDCALL FrameDoneCB(tPvFrame* pFrame)
{
	if (pFrame->Status == ePvErrSuccess)
		printf("Frame: %lu returned successfully\n", pFrame->FrameCount);
	else if (pFrame->Status == ePvErrDataMissing)
		//Possible improper network card settings. See GigE Installation Guide.
		printf("Frame: %lu dropped packets\n", pFrame->FrameCount);
	else if (pFrame->Status == ePvErrCancelled)
		printf("Frame cancelled %lu\n", pFrame->FrameCount);
	else
		printf("Frame: %lu Error: %u\n", pFrame->FrameCount, pFrame->Status);
    
	// if frame hasn't been cancelled, requeue frame
    if( AVTCamera::vecCam.size() == 1 ){

        AVTCamera* cam = AVTCamera::vecCam[0];

        cam->f_tmp = pFrame->FrameCount;        

        cam->lock();
        // convertRGB24toGray(static_cast<unsigned char*>(pFrame->ImageBuffer), cam->data, cam->width, cam->height);
        for( int i = 0; i < cam->nPix(); i++ ){
            cam->data[i] = static_cast<unsigned char*>(pFrame->ImageBuffer) + i;
        }
        cam->unlock();
        
        if(pFrame->Status != ePvErrCancelled){
            PvCaptureQueueFrame(cam->Handle,pFrame,FrameDoneCB);
        }
        
    }
}

/*-------------------------------*/
/* non-static methods            */
/*-------------------------------*/

AVTCamera::AVTCamera(const std::string& paramPath)
: ICamera<unsigned char>(1, 1, 8, 100), UID(0), Abort(false){
    
    if( !vecCam.empty() )
        throw "AVTCamera cannnot be allocated twice.\n";
    
    memset((void*)&(this->Handle), 0, sizeof(tPvHandle) );
    memset((void*)&(this->Frames), 0, sizeof(tPvFrame)*FRAMESCOUNT );
    
    if( PvInitialize() != ePvErrSuccess)
        throw "Failed to initialize AVTCamera.";
    
    // set ctrl+c handler
    SetConsoleCtrlHandler(CtrlCHandler, 1);
    
    // wait for a camera to be plugged in
    WaitForCamera();
    
    // get camera
    if( !this->CameraGet() || Abort){
        throw "camera not found.\n";
    }else{
        std::cout << "found camera connected.\n";
    }
    
    // load camera parameter file.
    if( !this->SetupLoad(paramPath.c_str()))
        throw "failed to load param file.";
    
    // setup camera
    if(!this->CameraSetup())
        throw "camera setup failed.\n";
    
    // set up parameters of camera.
    char buf[128];
    tPvAttributeInfo inf;
    if( PvAttrInfo(this->Handle, "Height", &inf) == ePvErrSuccess ){
        Value2String("Height", inf.Datatype, buf, 128);
        *const_cast<int*>(&this->height) = atoi(buf);
    }
    if( PvAttrInfo(this->Handle, "Width", &inf) == ePvErrSuccess ){
        Value2String("Width", inf.Datatype, buf, 128);
        *const_cast<int*>(&this->width) = atoi(buf);
    }
    if( PvAttrInfo(this->Handle, "FrameRate", &inf) == ePvErrSuccess ){
        Value2String("FrameRate", inf.Datatype, buf, 128);
        this->fps = atof(buf);
    }
    cout << "width:" << width << " height:" << height << " fps:" << fps << endl;
    
    // delete inner buffer and set up pointer
    delete this->data;
    this->data = new unsigned char*[this->nPix()];
    for( int i = 0; i < this->nPix(); i++ ){
        this->data[i] = new unsigned char;
    }

    initLock(&(this->mlock));
    
    vecCam.push_back(this);
    
}

// wait for a camera to be plugged in
void AVTCamera::WaitForCamera()
{
    printf("Waiting for a camera");
    while((PvCameraCount() == 0) && !this->Abort)
	{
		printf(".");
		Sleep(250);
	}
    printf("\n");
}


// trim the supplied string left and right
char* AVTCamera::strtrim(char *aString)
{
    int i;
    int lLength = static_cast<int>(strlen(aString));
    char* lOut = aString;
    
    // trim right
    for(i=lLength-1;i>=0;i--)
        if(isspace(aString[i]))
            aString[i]='\0';
        else
            break;
    
    lLength = static_cast<int>(strlen(aString));
    
    // trim left
    for(i=0;i<lLength;i++)
        if(isspace(aString[i]))
            lOut = &aString[i+1];
        else
            break;
    
    return lOut;
}

// get the first camera found
bool AVTCamera::CameraGet()
{
    tPvUint32 count,connected;
    tPvCameraInfoEx list;
    
    //regardless if connected > 1, we only set UID of first camera in list
	count = PvCameraListEx(&list,1,&connected, sizeof(tPvCameraInfoEx));
    if(count == 1)
    {
        this->UID = list.UniqueId;
        printf("Got camera %s\n",list.SerialNumber);
        return true;
    }
    else
	{
		printf("CameraGet: Failed to find a camera\n");
		return false;
	}
}

// open camera, allocate memory
// return value: true == success, false == fail
bool AVTCamera::CameraSetup()
{
    tPvErr errCode;
	bool failed = false;
	unsigned long FrameSize = 0;
    
	//open camera
	if ((errCode = PvCameraOpen(this->UID,ePvAccessMaster,&(this->Handle))) != ePvErrSuccess)
	{
		if (errCode == ePvErrAccessDenied)
			printf("PvCameraOpen returned ePvErrAccessDenied:\nCamera already open as Master, or camera wasn't properly closed and still waiting to HeartbeatTimeout.");
		else
			printf("PvCameraOpen err: %u\n", errCode);
		return false;
	}
    
	// Calculate frame buffer size
    if((errCode = PvAttrUint32Get(this->Handle,"TotalBytesPerFrame",&FrameSize)) != ePvErrSuccess)
	{
		printf("CameraSetup: Get TotalBytesPerFrame err: %u\n", errCode);
		return false;
	}
    
	// allocate the frame buffers
    for(int i=0;i<FRAMESCOUNT && !failed;i++)
    {
        this->Frames[i].ImageBuffer = new char[FrameSize];
        if(this->Frames[i].ImageBuffer)
        {
			this->Frames[i].ImageBufferSize = FrameSize;
		}
        else
		{
			printf("CameraSetup: Failed to allocate buffers.\n");
			failed = true;
		}
    }
    
    
	return !failed;
}

// close camera, free memory.
void AVTCamera::CameraUnsetup()
{
    tPvErr errCode;
	
    if((errCode = PvCameraClose(this->Handle)) != ePvErrSuccess)
	{
		printf("CameraUnSetup: PvCameraClose err: %u\n", errCode);
	}
	else
	{
		printf("Camera closed.\n");
	}
	// delete image buffers
    for(int i=0;i<FRAMESCOUNT;i++)
        delete [] (char*)this->Frames[i].ImageBuffer;
    
    this->Handle = NULL;
}

// set camera or driver attribute based on string value
bool AVTCamera::String2Value(const char* aLabel,tPvDatatype aType,char* aValue)
{
    switch(aType)
    {
        case ePvDatatypeString:
        {
            return (PvAttrStringSet(this->Handle,aLabel,aValue) == ePvErrSuccess);
        }
        case ePvDatatypeEnum:
        {
            return (PvAttrEnumSet(this->Handle,aLabel,aValue) == ePvErrSuccess);
        }
        case ePvDatatypeUint32:
        {
            tPvUint32 lValue = atol(aValue);
            tPvUint32 lMin,lMax;
            
            if(PvAttrRangeUint32(this->Handle,aLabel,&lMin,&lMax) == ePvErrSuccess)
            {
                if(lMin > lValue)
                    lValue = lMin;
                else
                    if(lMax < lValue)
                        lValue = lMax;
                
                return PvAttrUint32Set(this->Handle,aLabel,lValue) == ePvErrSuccess;
            }
            else
                return false;
        }
        case ePvDatatypeFloat32:
        {
            tPvFloat32 lValue = (tPvFloat32)atof(aValue);
            tPvFloat32 lMin,lMax;
            
            if(PvAttrRangeFloat32(this->Handle,aLabel,&lMin,&lMax) == ePvErrSuccess)
            {
                if(lMin > lValue)
                    lValue = lMin;
                else
                    if(lMax < lValue)
                        lValue = lMax;
                
                return PvAttrFloat32Set(this->Handle,aLabel,lValue) == ePvErrSuccess;
            }
            else
                return false;
        }
        case ePvDatatypeBoolean:
        {
            if(!(strcmp(aValue,"true")))
                return PvAttrBooleanSet(this->Handle,aLabel,true) == ePvErrSuccess;
            else
                return PvAttrBooleanSet(this->Handle,aLabel,false) == ePvErrSuccess;
        }
        default:
            return false;
    }
}

// encode the value of a given attribute in a string
bool AVTCamera::Value2String(const char* aLabel,tPvDatatype aType,char* aString,unsigned long aLength)
{
    switch(aType)
    {
        case ePvDatatypeString:
        {
            return PvAttrStringGet(this->Handle,aLabel,aString,aLength,NULL) == ePvErrSuccess;
        }
        case ePvDatatypeEnum:
        {
            return PvAttrEnumGet(this->Handle,aLabel,aString,aLength,NULL) == ePvErrSuccess;
        }
        case ePvDatatypeUint32:
        {
            tPvUint32 lValue;
            
            if(PvAttrUint32Get(this->Handle,aLabel,&lValue) == ePvErrSuccess)
            {
                sprintf_s(aString, aLength, "%lu",lValue);
                return true;
            }
            else
                return false;
            
        }
        case ePvDatatypeFloat32:
        {
            tPvFloat32 lValue;
            
            if(PvAttrFloat32Get(this->Handle,aLabel,&lValue) == ePvErrSuccess)
            {
                sprintf_s(aString, aLength, "%g",lValue);
                return true;
            }
            else
                return false;
        }
        case ePvDatatypeBoolean:
        {
            tPvBoolean lValue;
            
            if(PvAttrBooleanGet(this->Handle,aLabel,&lValue) == ePvErrSuccess)
            {
                if(lValue)
                    strcpy_s(aString, aLength, "true");
                else
                    strcpy_s(aString, aLength, "false");
                
                return true;
            }
            else
                return false;
        }
        default:
            return false;
    }
}

// write a given attribute in a text file
void AVTCamera::WriteAttribute(const char* aLabel,FILE* aFile)
{
    tPvAttributeInfo lInfo;
    
    if(PvAttrInfo(this->Handle,aLabel,&lInfo) == ePvErrSuccess)
    {
        if(lInfo.Datatype != ePvDatatypeCommand &&
           (lInfo.Flags & ePvFlagWrite))
        {
            char lValue[128];
            
            //get attribute
			if(Value2String(aLabel,lInfo.Datatype,lValue,128))
                fprintf(aFile,"%s = %s\n",aLabel,lValue);
            else
                fprintf(stderr,"attribute %s couldn't be saved\n",aLabel);
        }
    }
}

// read the attribute from text file
void AVTCamera::ReadAttribute(char* aLine)
{
    char* lValue = strchr(aLine,'=');
    char* lLabel;
    
    if(lValue)
    {
        lValue[0] = '\0';
        lValue++;
        
        lLabel = strtrim(aLine);
        lValue = strtrim(lValue);
        
        if(strlen(lLabel) && strlen(lValue))
        {
            tPvAttributeInfo lInfo;
            
            if(PvAttrInfo(this->Handle,lLabel,&lInfo) == ePvErrSuccess)
            {
                if(lInfo.Datatype != ePvDatatypeCommand &&
                   (lInfo.Flags & ePvFlagWrite))
                {
                    //set attribute
					if(!String2Value(lLabel,lInfo.Datatype,lValue))
                        fprintf(stderr,"attribute %s couldn't be loaded\n",lLabel);
                }
            }
        }
    }
}

// load the setup of a camera from the given file
bool AVTCamera::SetupLoad(const char* aFile)
{
    FILE* lFile = NULL;
    
#ifdef _WINDOWS
    fopen_s(&lFile, aFile, "r");
#else
    lFile = fopen(aFile,"r");
#endif
    
    if(lFile)
    {
        char lLine[256];
        
        while(!feof(lFile))
        {
            //read attributes from file
			if(fgets(lLine,256,lFile))
                ReadAttribute(lLine);
        }
        
        fclose(lFile);
        
        return true;
    }
    else
        return false;
}

// save the setup of a camera from the given file
bool AVTCamera::SetupSave(const char* aFile)
{
    FILE* lFile = NULL;
    
#ifdef _WINDOWS
    fopen_s(&lFile, aFile, "w+");
#else
    lFile = fopen(aFile,"w+");
#endif
    
    if(lFile)
    {
        bool            lRet = true;
        tPvAttrListPtr  lAttrs;
        tPvUint32       lCount;
        
        //Get all attributes
		if(PvAttrList(this->Handle,&lAttrs,&lCount) == ePvErrSuccess)
        {
            //Write attributes to file
			for(tPvUint32 i=0;i<lCount;i++)
                WriteAttribute(lAttrs[i],lFile);
        }
        else
            lRet = false;
        
        fclose(lFile);
        
        return lRet;
    }
    else
        return false;
}

// setup and start streaming
// return value: true == success, false == fail
bool AVTCamera::CameraStart()
{
    tPvErr errCode;
	bool failed = false;
    
    // NOTE: This call sets camera PacketSize to largest sized test packet, up to 8228, that doesn't fail
	// on network card. Some MS VISTA network card drivers become unresponsive if test packet fails.
	// Use PvUint32Set(handle, "PacketSize", MaxAllowablePacketSize) instead. See network card properties
	// for max allowable PacketSize/MTU/JumboFrameSize.
	if((errCode = PvCaptureAdjustPacketSize(this->Handle,8228)) != ePvErrSuccess)
	{
		printf("CameraStart: PvCaptureAdjustPacketSize err: %u\n", errCode);
		return false;
	}
    
    // start driver capture stream
	if((errCode = PvCaptureStart(this->Handle)) != ePvErrSuccess)
	{
		printf("CameraStart: PvCaptureStart err: %u\n", errCode);
		return false;
	}
	
    // queue frames with FrameDoneCB callback function. Each frame can use a unique callback function
	// or, as in this case, the same callback function.
	for(int i=0;i<FRAMESCOUNT && !failed;i++)
	{
		if((errCode = PvCaptureQueueFrame(this->Handle,&(this->Frames[i]),FrameDoneCB)) != ePvErrSuccess)
		{
			printf("CameraStart: PvCaptureQueueFrame err: %u\n", errCode);
			// stop driver capture stream
			PvCaptureEnd(this->Handle);
			failed = true;
		}
	}
    
	if (failed)
		return false;
    
	// set the camera in freerun trigger, continuous mode, and start camera receiving triggers
	if((PvAttrEnumSet(this->Handle,"FrameStartTriggerMode","Freerun") != ePvErrSuccess) ||
       (PvAttrEnumSet(this->Handle,"AcquisitionMode","Continuous") != ePvErrSuccess) ||
       (PvCommandRun(this->Handle,"AcquisitionStart") != ePvErrSuccess))
	{
		printf("CameraStart: failed to set camera attributes\n");
		// clear queued frame
		PvCaptureQueueClear(this->Handle);
		// stop driver capture stream
		PvCaptureEnd(this->Handle);
		return false;
	}
    
	return true;
}


// stop streaming
void AVTCamera::CameraStop()
{
    tPvErr errCode;
	
	//stop camera receiving triggers
	if ((errCode = PvCommandRun(this->Handle,"AcquisitionStop")) != ePvErrSuccess)
		printf("AcquisitionStop command err: %u\n", errCode);
	else
		printf("AcquisitionStop success.\n");
    
	//PvCaptureQueueClear aborts any actively written frame with Frame.Status = ePvErrDataMissing
	//Further queued frames returned with Frame.Status = ePvErrCancelled
	
	//Add delay between AcquisitionStop and PvCaptureQueueClear
	//to give actively written frame time to complete
	Sleep(200);
	
	printf("Calling PvCaptureQueueClear...\n");
	if ((errCode = PvCaptureQueueClear(this->Handle)) != ePvErrSuccess)
		printf("PvCaptureQueueClear err: %u\n", errCode);
	else
		printf("...Queue cleared.\n");
    
	//stop driver stream
	if ((errCode = PvCaptureEnd(this->Handle)) != ePvErrSuccess)
		printf("PvCaptureEnd err: %u\n", errCode);
	else
		printf("Driver stream stopped.\n");
}

void AVTCamera::capture(){
    
    tPvUint32 exposureVal;
    
    if( this->state == standby ){
        if(!CameraStart()){
            throw "failed to start camera.\n";
        }
        this->state = run;
    }

    // check camera is plugged and not aborted
    if((PvAttrUint32Get(this->Handle, "ExposureValue", &exposureVal) == ePvErrUnplugged) || this->Abort == true){
        cout << "camera aborted" << endl;
        this->state = stop;
    }

}

void AVTCamera::lock(){ acquireLock( &(this->mlock) ); }

void AVTCamera::unlock(){ releaseLock( &(this->mlock) ); }

AVTCamera::~AVTCamera(){
    deleteLock(&(this->mlock));
    CameraStop();
    CameraUnsetup();
    this->data = NULL;
    PvUnInitialize();
}

#endif