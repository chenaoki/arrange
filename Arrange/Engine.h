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
        private:
            static std::string mkpath(const std::vector<string> &list_dir, const std::string& del, const int depth)
            {
                std::string ret("");
                for( int i = 0; i < list_dir.size() - depth; i++){
                    ret += del;
                    ret += list_dir[i];
                }
                return ret;
            };
            static void mkdir_p(const std::string& path, const std::string& del)
            {
                std::vector<string> list_dir;
                boost::split(list_dir, path, boost::is_any_of(del));
                int ret = -1;
                int depth = 0;
                do{
                    std::string p = mkpath(list_dir, del, depth);
                    ret = mkdir( p.c_str(), 0777 );
                }while(ret != 0 && ++depth < list_dir.size() - 1 );
                while( --depth >= 0 ){
                    std::string p = mkpath(list_dir, del, depth);
                    ret = mkdir( p.c_str(), 0777 );
                }
            };
        public:
            Engine(std::string& paramFilePath)
            {
                /* load param */
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                dstDir  = obj["dstDir"].get<std::string>();
                
                /* make output directory */
                std::string temp = this->dstDir;
                mkdir_p( temp, "/" );
                
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