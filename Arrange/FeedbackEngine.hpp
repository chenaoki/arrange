//
//  RealtimeEngine.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/10.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_RealtimeEngine_hpp
#define Arrange_RealtimeEngine_hpp

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
#include "MovieAnalyzer.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "picojson.h"
#include "Optical.h"
#include "Engine.h"

using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;
using namespace MLARR::Engine;

namespace MLARR{
	namespace Engine{
        
        template<typename T>
		class FeedbackEngine : Engine<T>{
            
            FeedbackEngine(std::string& paramFilePath) : Engine<T>(paramFilePath){};
            
            ~FeedbackEngine(){};
            
            void execute(void){
                
            };
            
        };
        
    }
}

#endif
