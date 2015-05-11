//
//  ColorbarEngine.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2015/05/11.
//  Copyright (c) 2015年 ARR. All rights reserved.
//

#ifndef Arrange_ColorbarEngine_hpp
#define Arrange_ColorbarEngine_hpp

#include "stdafx.h"

#include <vector>
#include <string>
#include <map>

#include "Basic.h"
#include "Analyzer.h"
#include "Hilbert.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "Optical.h"
#include "Engine.h"
#include "CameraFactory.hpp"

using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;
using namespace std;

namespace MLARR{
    namespace Engine{
        
        class ColorbarEngine : public Engine{
            
        private:
            std::map<string, ColorMap*> map_colmap;
            int width;
            int height;
            
        public:
            
            ColorbarEngine(std::string& paramFilePath) : Engine(paramFilePath)
            {
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath);
                width = atoi( obj["width"].to_str().c_str() );
                height = atoi( obj["height"].to_str().c_str() );
                map_colmap["orange"] = dynamic_cast<ColorMap*>(&colMap_orange);
                map_colmap["hsv"] = dynamic_cast<ColorMap*>(&colMap_hsv);
                map_colmap["gray"] = dynamic_cast<ColorMap*>(&colMap_gray);
            };
            
            ~ColorbarEngine(void){};
            
            void execute(void){
                Image<double> img(height, width, 0.0);
                for( int j = 0; j < height; j++){
                    for( int i = 0; i < width; i++){
                        img.setValue( i, j, (j+1) / static_cast<double>(height) );
                    }
                }
                for (map<string, ColorMap*>::iterator it = map_colmap.begin(); it != map_colmap.end(); it++) {
                    Display<double> disp ( "colorbar", img, 1.0, 0.0, *(it->second));
                    disp.show();
                    cvWaitKey(500);
                    disp.save(this->dstDir, string("%s/") + it->first + string(".jpg"));
                }
            }
            
        };

    }
}
#endif
