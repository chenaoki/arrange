#pragma once

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
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
                /* load param */
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                dstDir  = obj["dstDir"].get<std::string>();
                
                /* make output directory */
                std::string temp = this->dstDir;
                mkdir( temp.c_str() , 0777);
                
                /* copy parameter file to the dst directory. */
                ifstream ifs(paramFilePath.c_str());
                ofstream ofs((this->dstDir+string("/param.json")).c_str());
                ofs << ifs.rdbuf();
                ifs.close(); ofs.close();

            };
            
            virtual ~Engine(void){};
            
            virtual void execute(void) = 0;

            
        };
        
	}
}