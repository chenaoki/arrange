// main.cpp

#include "stdafx.h"

#include "Electrode.h"
#include "Engine.h"

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
		("engine,e", value<string>(), "Analysis engine")
		("param,p", value<string>(),"Parameter file")
	;

	variables_map values;

	try{

        store( parse_command_line( argc, argv, options ) , values );
		notify(values);
        
		engine = values["engine"].as<string>();
        paramFile = values["param"].as<string>();
        
		if( engine == std::string("optical")) {
			OpticalOfflineAnalysisEngine<unsigned short> eng(paramFile);
            eng.execute();
		}else{
			throw string("not implemented yet.");
		}
        
	}catch( std::exception& e ){
		std::cout << e.what();
	}
         
	return 0;
}
