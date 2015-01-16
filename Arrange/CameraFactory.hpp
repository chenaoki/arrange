//
//  CameraFactory.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/29.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_CameraFactory_hpp
#define Arrange_CameraFactory_hpp

#include "stdafx.h"

#include "Basic.h"
#include "IO.h"
#include "AVTCamera.h"

using namespace std;
using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;

namespace MLARR{
    namespace IO{
    
        class CameraFactory{
        public:
            static void* create( const std::string &camType, const std::string& paramFilePath){
                
                void* cam = NULL;
                
                if( string::npos != camType.find("raw") ){
                    
                    picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath);
                    picojson::object raw = obj["rawCamera"].get<picojson::object>();
                    string srcDir = raw["srcDir"].get<std::string>();
                    string format  = raw["format"].get<std::string>();
                    int size = atoi(raw["size"].to_str().c_str());
                    int frameRate = atoi(raw["frameRate"].to_str().c_str());
                    picojson::object& frm = raw["frame"].get<picojson::object>();
                    int f_start = atoi( frm["start"].to_str().c_str() );
                    int f_skip  = atoi( frm["skip"].to_str().c_str() );
                    int f_stop  = atoi( frm["stop"].to_str().c_str() );
                    
                    if("raw_dalsa" == camType ){
                        cam = static_cast<void*>( new DalsaRawFileCamera( srcDir, format, f_start, f_skip, f_stop, size, frameRate ));
                    }else if( "raw_hurricane" == camType ){
                        cam = static_cast<void*>( new HurricaneRawFileCamera( srcDir, format, f_start, f_skip, f_stop, size, frameRate ));
                    }else if( "raw_max" == camType ){
                        cam = static_cast<void*>( new MaxRawFileCamera( srcDir, format, f_start, f_skip, f_stop, size, frameRate ));
                    }else if( "raw_sa4" ){
                        cam = static_cast<void*>( new SA4RawFileCamera( srcDir, format, f_start, f_skip, f_stop, size, frameRate ));
                    }else if( "raw_avt" == camType ){
                        cam = static_cast<void*>( new Mono8RawFileCamera( srcDir, format, f_start, f_skip, f_stop, size, frameRate));
                    }else {
                        throw camType + string(": unknown unsigned short camera type.");
                    }
                
                }else{
                    
                    if("avt" == camType ){
                        
                        picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                        string camParamFile = obj["camParamFile"].get<std::string>();
                        
#ifdef USE_AVT_CAM
                        cam = static_cast<void*>( new AVTCamera( camParamFile ));
#else // USE_AVT_CAM
                        throw string("AVTCamera not available (check platform.)");
#endif // USE_AVT_CAM
                        
                    }else{
                        throw camType + string("unknown unsigned char camera type.");
                    }
                }
                
                return cam;
                
            };
        };
        
    }

}

#endif
