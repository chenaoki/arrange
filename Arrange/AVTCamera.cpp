//
//  AVTCamera.cpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/07.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#include "AVTCamera.h"
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef _OSX_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include "XGetopt.h"
#endif

#ifdef _OSX_
#define strncpy_s(dest,len,src,count) strncpy(dest,src,count)
#define sprintf_s(dest,len,format,args...) sprintf(dest,format,args)
#define sscanf_s sscanf
#define strcpy_s(dst,len,src) strcpy(dst,src)
#define _strdup(src) strdup(src)
#define strtok_s(tok,del,ctx) strtok(tok,del)
#endif

#if defined(_LINUX) || defined(_QNX) || defined(_OSX) || defined(_OSX_)
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <strings.h>
#endif

//#include "PvApi.h"



/*using namespace MLARR::IO;
using namespace MLARR::Basic;*/

static const int camBit = 12;
static const int default_camWidth = 640;
static const int default_camHeight = 480;
static const int default_fps = 200;

#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
// put the calling thread to sleep for a given amount of millisecond
void Sleep(unsigned int time)
{
    struct timespec t,r;
    
    t.tv_sec    = time / 1000;
    t.tv_nsec   = (time % 1000) * 1000000;
    
    while(nanosleep(&t,&r)==-1)
        t = r;
}
#endif

// display the usage details
void ShowUsage()
{
    printf("usage: CamSetup -u <camera unique ID>| -i <camera IP> -l|-s <file>\n");
    printf("-u\tcamera unique ID\n");
    printf("-i\tcamera IP address\n");
    printf("-l\tload setup\n");
    printf("-s\tsave setup\n");
}

//// trim the supplied string left and right
//char* strtrim(char *aString)
//{
//    int i;
//    int lLength = strlen(aString);
//    char* lOut = aString;
//    
//    // trim right
//    for(i=lLength-1;i>=0;i--)
//        if(isspace(aString[i]))
//            aString[i]='\0';
//        else
//            break;
//    
//    lLength = strlen(aString);
//    
//    // trim left
//    for(i=0;i<lLength;i++)
//        if(isspace(aString[i]))
//            lOut = &aString[i+1];
//        else
//            break;
//    
//    return lOut;
//}

//// set camera or driver attribute based on string value
//bool String2Value(tPvHandle aCamera,const char* aLabel,tPvDatatype aType,char* aValue)
//{
//    switch(aType)
//    {
//        case ePvDatatypeString:
//        {
//            return (PvAttrStringSet(aCamera,aLabel,aValue) == ePvErrSuccess);
//        }
//        case ePvDatatypeEnum:
//        {
//            return (PvAttrEnumSet(aCamera,aLabel,aValue) == ePvErrSuccess);
//        }
//        case ePvDatatypeUint32:
//        {
//            tPvUint32 lValue = atol(aValue);
//            tPvUint32 lMin,lMax;
//            
//            if(PvAttrRangeUint32(aCamera,aLabel,&lMin,&lMax) == ePvErrSuccess)
//            {
//                if(lMin > lValue)
//                    lValue = lMin;
//                else
//                    if(lMax < lValue)
//                        lValue = lMax;
//                
//                return PvAttrUint32Set(aCamera,aLabel,lValue) == ePvErrSuccess;
//            }
//            else
//                return false;
//        }
//        case ePvDatatypeFloat32:
//        {
//            tPvFloat32 lValue = (tPvFloat32)atof(aValue);
//            tPvFloat32 lMin,lMax;
//            
//            if(PvAttrRangeFloat32(aCamera,aLabel,&lMin,&lMax) == ePvErrSuccess)
//            {
//                if(lMin > lValue)
//                    lValue = lMin;
//                else
//                    if(lMax < lValue)
//                        lValue = lMax;
//                
//                return PvAttrFloat32Set(aCamera,aLabel,lValue) == ePvErrSuccess;
//            }
//            else
//                return false;
//        }
//        case ePvDatatypeBoolean:
//        {
//            if(!(strcmp(aValue,"true")))
//                return PvAttrBooleanSet(aCamera,aLabel,true) == ePvErrSuccess;
//            else
//                return PvAttrBooleanSet(aCamera,aLabel,false) == ePvErrSuccess;
//        }
//        default:
//            return false;
//    }
//}
//
//// encode the value of a given attribute in a string
//bool Value2String(tPvHandle aCamera,const char* aLabel,tPvDatatype aType,char* aString,unsigned long aLength)
//{
//    switch(aType)
//    {
//        case ePvDatatypeString:
//        {
//            return PvAttrStringGet(aCamera,aLabel,aString,aLength,NULL) == ePvErrSuccess;
//        }
//        case ePvDatatypeEnum:
//        {
//            return PvAttrEnumGet(aCamera,aLabel,aString,aLength,NULL) == ePvErrSuccess;
//        }
//        case ePvDatatypeUint32:
//        {
//            tPvUint32 lValue;
//            
//            if(PvAttrUint32Get(aCamera,aLabel,&lValue) == ePvErrSuccess)
//            {
//                sprintf_s(aString, aLength, "%lu",lValue);
//                return true;
//            }
//            else
//                return false;
//            
//        }
//        case ePvDatatypeFloat32:
//        {
//            tPvFloat32 lValue;
//            
//            if(PvAttrFloat32Get(aCamera,aLabel,&lValue) == ePvErrSuccess)
//            {
//                sprintf_s(aString, aLength, "%g",lValue);
//                return true;
//            }
//            else
//                return false;
//        }
//        case ePvDatatypeBoolean:
//        {
//            tPvBoolean lValue;
//            
//            if(PvAttrBooleanGet(aCamera,aLabel,&lValue) == ePvErrSuccess)
//            {
//                if(lValue)
//                    strcpy_s(aString, aLength, "true");
//                else
//                    strcpy_s(aString, aLength, "false");
//                
//                return true;
//            }
//            else
//                return false;
//        }
//        default:
//            return false;
//    }
//}
//
//// write a given attribute in a text file
//void WriteAttribute(tPvHandle aCamera,const char* aLabel,FILE* aFile)
//{
//    tPvAttributeInfo lInfo;
//    
//    if(PvAttrInfo(aCamera,aLabel,&lInfo) == ePvErrSuccess)
//    {
//        if(lInfo.Datatype != ePvDatatypeCommand &&
//           (lInfo.Flags & ePvFlagWrite))
//        {
//            char lValue[128];
//            
//            //get attribute
//			if(Value2String(aCamera,aLabel,lInfo.Datatype,lValue,128))
//                fprintf(aFile,"%s = %s\n",aLabel,lValue);
//            else
//                fprintf(stderr,"attribute %s couldn't be saved\n",aLabel);
//        }
//    }
//}
//
//// read the attribute from text file
//void ReadAttribute(tPvHandle aCamera,char* aLine)
//{
//    char* lValue = strchr(aLine,'=');
//    char* lLabel;
//    
//    if(lValue)
//    {
//        lValue[0] = '\0';
//        lValue++;
//        
//        lLabel = strtrim(aLine);
//        lValue = strtrim(lValue);
//        
//        if(strlen(lLabel) && strlen(lValue))
//        {
//            tPvAttributeInfo lInfo;
//            
//            if(PvAttrInfo(aCamera,lLabel,&lInfo) == ePvErrSuccess)
//            {
//                if(lInfo.Datatype != ePvDatatypeCommand &&
//                   (lInfo.Flags & ePvFlagWrite))
//                {
//                    //set attribute
//					if(!String2Value(aCamera,lLabel,lInfo.Datatype,lValue))
//                        fprintf(stderr,"attribute %s couldn't be loaded\n",lLabel);
//                }
//            }
//        }
//    }
//}
//
//// load the setup of a camera from the given file
//bool SetupLoad(tPvHandle aCamera,const char* aFile)
//{
//    FILE* lFile = NULL;
//    
//#ifdef _WINDOWS
//    fopen_s(&lFile, aFile, "r");
//#else
//    lFile = fopen(aFile,"r");
//#endif
//    
//    if(lFile)
//    {
//        char lLine[256];
//        
//        while(!feof(lFile))
//        {
//            //read attributes from file
//			if(fgets(lLine,256,lFile))
//                ReadAttribute(aCamera,lLine);
//        }
//        
//        fclose(lFile);
//        
//        return true;
//    }
//    else
//        return false;
//}

//// save the setup of a camera from the given file
//bool SetupSave(tPvHandle aCamera,const char* aFile)
//{
//    FILE* lFile = NULL;
//    
//#ifdef _WINDOWS
//    fopen_s(&lFile, aFile, "w+");
//#else
//    lFile = fopen(aFile,"w+");
//#endif
//    
//    if(lFile)
//    {
//        bool            lRet = true;
//        tPvAttrListPtr  lAttrs;
//        tPvUint32       lCount;
//        
//        //Get all attributes
//		if(PvAttrList(aCamera,&lAttrs,&lCount) == ePvErrSuccess)
//        {
//            //Write attributes to file
//			for(tPvUint32 i=0;i<lCount;i++)
//                WriteAttribute(aCamera,lAttrs[i],lFile);
//        }
//        else
//            lRet = false;
//        
//        fclose(lFile);
//        
//        return lRet;
//    }
//    else
//        return false;
//}
//
//
//AVTCamera::AVTCamera(const std::string& paramFilePath)
//: ICamera<unsigned short>(default_camWidth, default_camHeight, camBit, default_fps){
//    
//    
//    
//}
//
//AVTCamera::~AVTCamera(){
//    
//}