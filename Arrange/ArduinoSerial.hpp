//
//  Serial.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/09/04.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_Serial_hpp
#define Arrange_Serial_hpp

#ifdef _OSX

#include "stdafx.h"

#define LINE_LENGTH_MAX 64
#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

namespace MLARR {
    namespace IO{
        
        class ArduinoSerial{
        public:
            static int fd;
            static char readbuf[LINE_LENGTH_MAX];
            static struct termios oldtio, newtio;
            static int isrunning;
            static const int enq = 0x05; // ENQuiry in ASCII table
        public:
            static int Open(const std::string& modeDevice){
                
                isrunning = FALSE;
                
                /* 0_RDRW: readable & writable */
                /* O_NOCTTY: avoid tty control not-intended ctrl-c caused by noise */
                /* allow to access a serical interface via fd */
                
                fd = open(modeDevice.c_str(), O_RDWR | O_NOCTTY );
                
                if (fd <0) {
                    printf("open error: %s \n", modeDevice.c_str() );
                    return -1;
                }
                
                tcgetattr(fd,&oldtio); /* save current port settings */
                bzero(&newtio,  sizeof(newtio));
                newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
                
                /*
                 BAUDRATE: baudrate setting
                 CRTSCTS : output hardware flow control
                 CS8     : 8n1 (8 bits, non parity, stop bit 1)
                 CLOCAL  : local connection, model is not used
                 CREAD   : invalid receiving characters
                 */
                
                newtio.c_iflag = IGNPAR | ICRNL;
                
                /*
                 IGNPAR : ignore parity error data
                 ICRNL  : CR is set to NL
                 */
                
                newtio.c_oflag = 0;
                
                /*
                 ICANON  : カノニカル入力を有効にする
                 全てのエコーを無効にし，プログラムに対してシグナルは送らせない
                */
                newtio.c_lflag = ICANON;
                
                /* raw mode ouput */
                
                /* set input mode (non-canonical, no echo,...) */
                //  newtio.c_lflag = ICANON; /* input modes : non-canonical, or ICANON is used ? */
                newtio.c_lflag = 0; /* 0 : non-canonical, or ICANON */
                /* c_cc[MCCS] = control chars */
                newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
                //  newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */
                newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
                tcflush(fd, TCIFLUSH); /* discard the data that is recieved but not read. */
                tcsetattr(fd,TCSANOW,&newtio); /* TCSANOW = right now */
                
                isrunning = TRUE;
                return 0;
            };
            
            static void Close(void){
                tcsetattr(fd,TCSANOW,&oldtio); //restore previous port setting
                close(fd);
            };
            
            static void Interrupt(int sig){ isrunning = FALSE; };
            
            static void ReadLine(string& s){
                
                char result;
                int i = 0;
                tcflush(fd, TCIFLUSH); //clear the receiving buffer
                
                while(read(fd,&result,1) > 0)
                {
                    if ( result == '\r') break;
                    if ( result == '\n') break;
                    readbuf[i] = result;
                    if( ++i >= LINE_LENGTH_MAX -1 ) break;
                }
                readbuf[i] = '\0';
                s = string(readbuf);
            
            };
            
            static int WriteLine(string& s){
                string msg = s + string("\n");
                return static_cast<int>( write(fd, msg.c_str(), msg.length()));
            };
            
            static int PutChar(char* cptr){
                return static_cast<int>( write(fd, cptr, 1) );
            };
            
            static int PutInt(const int* const s){
                if(write(fd,s,sizeof(int)) != 1)
                    return -1;
                return 0;
            };
        };
        
    }
}

#endif //_OSX


#endif
