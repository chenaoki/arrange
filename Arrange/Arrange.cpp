// main.cpp

#include "stdafx.h"

#include "Electrode.h"
#include "Engine.h"

int main(int argc, char* argv[])
{
	using namespace std;
    using namespace boost::program_options;
    using namespace MLARR::Engine;

    string exeMode;
    string camType;
	string paramFile;
    
    cvDestroyAllWindows();
	
    // Parsing options.
	options_description options("options");
	options.add_options()
		("online,o", "Online mode")
		("menu,m", value<string>(),"Execution mode")
		("param,p", value<string>(),"Param file")
	;

	variables_map values;

	try{

        store( parse_command_line( argc, argv, options ) , values );
		notify(values);
		exeMode = values["menu"].as<string>();
        paramFile = values["param"].as<string>();
		
//		if( !values.count("online") ){
			
            EngineOffLine<unsigned short> eng(paramFile);
            
            if( "full" == exeMode || "revnorm" == exeMode  )
                eng.revNorm();
            if( "full" == exeMode || "hilbert" == exeMode  )
                eng.hilbertPhase();
            if( "full" == exeMode || "monitor" == exeMode  )
                eng.monitorElecPhase();
            if( "full" == exeMode || "ps" == exeMode  )
                eng.phaseSingularityAnalysis();
/*
		}else{

			throw string("not implemented yet.");

		}
*/
	}catch( std::exception& e ){
		std::cout << e.what();
	}
         
	return 0;
}
