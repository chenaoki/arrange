// main.cpp

#include "stdafx.h"

#include "Electrode.h"

#include "OpticalOfflineAnalysisEngine.hpp"
#include "FeedbackEngine.hpp"
#include "MultiStimSetupEngine.hpp"

int main(int argc, char* argv[])
{
	using namespace std;
    using namespace boost::program_options;
    using namespace MLARR::Engine;

    string engine;
	string paramFile;
    
    cvDestroyAllWindows();
	
    // Parsing options.
	options_description options("options");
	options.add_options()
        ("mode,m", value<string>(),   "Execution mode")
		("engine,e", value<string>(), "Analysis engine")
		("param,p", value<string>(),  "Parameter file")
	;

	variables_map values;

    Engine *eng = nullptr;
    
    
	try{

        store( parse_command_line( argc, argv, options ) , values );
		notify(values);
        
		engine = values["engine"].as<string>();
        paramFile = values["param"].as<string>();
        
		if( engine == std::string("optical")) {
            eng = dynamic_cast<Engine*>(new OpticalOfflineAnalysisEngine<unsigned short>(paramFile));
		}else if( engine == string("feedback") ){
            /* create camera object */
            picojson::object obj = MLARR::IO::loadJsonParam(paramFile) ;
            string camType = obj["camType"].get<std::string>();
            if( camType == "avt" ){
                eng = dynamic_cast<Engine*>(new FeedbackEngine<unsigned char>(paramFile));
            }
        }else if( engine == string("mssetup")){
            eng = dynamic_cast<Engine*>(new MultiStimSetupEngine(paramFile));
        }else{
			throw engine + string(" engine is not implemented yet.");
		}
        
        if( !eng ){
            throw string("engine not created.");
        }        
        eng->execute();
        
	}catch( std::exception& e ){
		std::cout << e.what();
	}catch( std::string& str){
        std::cout << str << endl;
    }
    
    if(eng) delete eng;
	return 0;
}
