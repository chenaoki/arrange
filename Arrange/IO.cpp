//
//  IO.cpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/07.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#include "IO.h"
#include "ArduinoSerial.hpp"

using namespace MLARR::IO;

HSVColorMap MLARR::IO::colMap_hsv;
OrangeColorMap MLARR::IO::colMap_orange;
GrayColorMap MLARR::IO::colMap_gray;

picojson::object MLARR::IO::loadJsonParam(const std::string& paramFilePath){
    
    std::ifstream ifs(paramFilePath.c_str());
    if( !ifs.is_open() ){
        throw string("failed to open parameter file.");
    }
    char* buf = new char[1024];
    std::string str_json("");
    while (!ifs.eof()) {
        ifs.getline(buf, 1024);
        str_json += std::string( buf );
    }
    ifs.close();
    
    picojson::value val;
    std::string  err;
    picojson::parse(val, str_json.c_str(), str_json.c_str() + str_json.size(), &err);
    if( !err.empty() ){
        throw "failed to parse json";
    }
    
    return val.get<picojson::object>();
    
};

int ArduinoSerial::fd;
struct termios ArduinoSerial::newtio;
struct termios ArduinoSerial::oldtio;
int ArduinoSerial::isrunning;
char ArduinoSerial::readbuf[LINE_LENGTH_MAX];

