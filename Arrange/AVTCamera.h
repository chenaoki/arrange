//
//  AVTCamera.h
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/07.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_AVTCamera_h
#define Arrange_AVTCamera_h
#ifdef USE_AVT_CAM

#include "IO.h"
#include "PvApi.h"
#include "Lock.h"

#ifdef _WINDOWS
#define _STDCALL __stdcall
#else
#define _STDCALL
#endif

namespace MLARR {
    
    namespace IO {
    
        static const int FRAMESCOUNT = 15;
        
        class AVTCamera : public ICamera<unsigned char>{
        public:
            static          std::vector<AVTCamera*> vecCam;
            unsigned long   UID;
            tPvHandle       Handle;
            tPvFrame        Frames[FRAMESCOUNT];
            bool            Abort;
            tLock           mlock;
        private:
/*
#ifdef _WINDOWS
            BOOL WINAPI CtrlCHandler(DWORD dwCtrlType);
#else
            void CtrlCHandler(int Signo);
#endif
*/
            void WaitForCamera();
            char* strtrim(char *aString);
            bool CameraGet();
            bool CameraSetup();
            void CameraUnsetup();
            bool CameraStart();
            void CameraStop();
            bool String2Value(const char* aLabel,tPvDatatype aType,char* aValue);
            bool Value2String(const char* aLabel,tPvDatatype aType,char* aString,unsigned long aLength);
            void WriteAttribute(const char* aLabel,FILE* aFile);
            void ReadAttribute(char* aLine);
            bool SetupLoad(const char* aFile);
            bool SetupSave(const char* aFile);
        public:
            AVTCamera(const std::string& paramPath);
            ~AVTCamera(void);
            void lock();
            void unlock();
            void capture(void);
        };
    }
}

#endif
#endif
