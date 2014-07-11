// MLARR_LIB.cpp : メイン プロジェクト ファイルです。

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
	string srcDir;
	string srcFmt;
	string dstDir;
	string elecSet;
	string pinStim;
	int f_begin, f_interval, f_end;
    
    cvNamedWindow("window",1);
    cvWaitKey(1000);
    cvDestroyAllWindows();
	
    // Parsing options.
	options_description options("options");
	options.add_options()
		("online,o", "Online mode")
		("menu,m", value<string>(),"exeMode")
		("cam,c", value<string>(),"camType")
		("src,s", value<string>(),"srcDir")
		("fmt,f", value<string>(),"srcFmt")
		("dst,d", value<string>(),"dstDir")
		("pins,p", value<string>(),"pinStim")
		("begin,b", value<int>()->default_value(1),"f_begin")
		("end,e", value<int>()->default_value(100),"f_end")
		("interval,i", value<int>()->default_value(1),"f_interval")
	;

	variables_map values;

	try{

        store( parse_command_line( argc, argv, options ) , values );
		notify(values);
		exeMode = values["menu"].as<string>();
		camType = values["cam"].as<string>();
		srcDir = values["src"].as<string>();
		srcFmt = values["fmt"].as<string>();
		dstDir = values["dst"].as<string>();
		f_begin = values["begin"].as<int>();
		f_end = values["end"].as<int>();
		f_interval = values["interval"].as<int>();
		
		std::vector<MLARR::IO::Electrode> elecs;
		if( values.count("pins") ){
			pinStim = values["pins"].as<string>();
			MLARR::IO::Electrode::loadElectrodeSetting( pinStim, elecs );
		}

		if( !values.count("online") ){
			
			if( camType == "dalsa" ){
				DalsaRawFileCamera cam( srcDir, srcFmt, f_begin, f_interval, f_end);
				EngineOffLine<DalsaRawFileCamera> eng( cam, elecs, dstDir );
				
				if( "full" == exeMode || "revnorm" == exeMode  )
					eng.revNorm();
				if( "full" == exeMode || "hilbert" == exeMode  )
					eng.hilbertPhase();
				if( "full" == exeMode || "monitor" == exeMode  )
					eng.monitorElecPhase();
				if( "full" == exeMode || "ps" == exeMode  )
					eng.phaseSingularityAnalysis();

			}else if( camType == "max" ){
				MaxRawFileCamera cam( srcDir, srcFmt, f_begin, f_interval, f_end);
				EngineOffLine<MaxRawFileCamera> eng( cam, elecs, dstDir );
				
				if( "full" == exeMode || "revnorm" == exeMode  )
					eng.revNorm();
				if( "full" == exeMode || "hilbert" == exeMode  )
					eng.hilbertPhase();
				if( "full" == exeMode || "monitor" == exeMode  )
					eng.monitorElecPhase();
				if( "full" == exeMode || "ps" == exeMode  )
					eng.phaseSingularityAnalysis();
			}
			
		}else{

			throw string("not implemented yet.");

		}

	}catch( std::exception& e ){
		std::cout << e.what();
	}
         
	return 0;
}
