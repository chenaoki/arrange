#pragma once

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
#include "MovieAnalyzer.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "Optical.h"
#include "CameraFactory.hpp"

using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;

namespace MLARR{
	namespace Engine{
        
        class Engine{
            
		protected:
            std::string dstDir;
        public:
            Engine(std::string& paramFilePath)
            {
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                dstDir  = obj["dstDir"].get<std::string>();
            };
            
            virtual ~Engine(void){};
            
            virtual void execute(void) = 0;

            
        };
        
	}
}