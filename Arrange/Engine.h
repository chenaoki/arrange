#pragma once

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
#include "MovieAnalyzer.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "picojson.h"
#include "Optical.h"

using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;

namespace MLARR{
	namespace Engine{
        
		const std::string fmt_raw_roi("%s/raw/roi.raw");
		const std::string fmt_raw_roh("%s/raw/roi_q.raw");
		const std::string fmt_raw_max("%s/raw/max/max_%05d.raw");
		const std::string fmt_raw_min("%s/raw/min/min_%05d.raw");
		const std::string fmt_raw_opt("%s/raw/opt/opt_%05d.raw");
		const std::string fmt_raw_hbt("%s/raw/hbt/hbt_%05d.raw");
		const std::string fmt_jpg_max("%s/jpg/max/max_%05d.jpg");
		const std::string fmt_jpg_min("%s/jpg/min/min_%05d.jpg");
		const std::string fmt_jpg_opt("%s/jpg/opt/opt_%05d.jpg");
		const std::string fmt_jpg_hbt("%s/jpg/hbt/hbt_%05d.jpg");
		const std::string fmt_jpg_hbf("%s/jpg/hbf/hbf_%05d.jpg");
		const std::string fmt_jpg_psh("%s/jpg/psh/psh_%05d.jpg");
		const std::string fmt_jpg_psp("%s/jpg/psp/psp_%05d.jpg");
		const std::string fmt_log_phs("%s/log/phs.log");
        const std::string fmt_log_psp("%s/log/psp.log");

        template<typename T>
        class Engine{
            
		protected:
			std::string camType;
            std::string dirPath;
            std::string format;
            std::string dstDir;
            int f_start;
            int f_skip;
            int f_stop;
        public:
            Engine(std::string& paramFilePath)
            {
                std::ifstream ifs(paramFilePath.c_str());
                if( !ifs.is_open() ){
                    throw "failed to open parameter file.";
                }
                char* buf = new char[1024];
                string str_json("");
                while (!ifs.eof()) {
                    ifs.getline(buf, 1024);
                    str_json += string( buf );
                }
                ifs.close();

                picojson::value val;
                std::string  err;
                picojson::parse(val, str_json.c_str(), str_json.c_str() + str_json.size(), &err);
                if( !err.empty() ){
                    throw "failed to parse json";
                }
                
                picojson::object& obj = val.get<picojson::object>();
                camType = obj["camType"].get<std::string>();
                dstDir  = obj["dstDir"].get<std::string>();
                dirPath = obj["dirPath"].get<std::string>();
                format  = obj["format"].get<std::string>();
                picojson::object& frm = obj["frame"].get<picojson::object>();
                f_start = atoi( frm["start"].to_str().c_str() );
                f_skip  = atoi( frm["skip"].to_str().c_str() );
                f_stop  = atoi( frm["stop"].to_str().c_str() );
                
                /* make output directories */
                std::string temp = this->dstDir;
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/raw";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/raw/max";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/raw/min";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/raw/opt";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/raw/hbt";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/max";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/min";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/opt";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/hbt";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/hbf";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/psh";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/psp";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/log";
                mkdir( temp.c_str() , 0777);

            };
            
            virtual ~Engine(void){};
            
            virtual void execute(void) = 0;

            
        };
        
        template<typename T>
		class OpticalOfflineAnalysisEngine : Engine<T>{

		private:
            RawFileCamera<T>* cam;
            std::vector<std::string> vec_menu;
            std::vector<MLARR::IO::Electrode> _electrodes;
            unsigned short rnm_minDelta;
            int hbt_size;
            int hbt_minPeakDistance;
            int hbt_filSize;
            int psp_openRoi;
            int psp_openPS;
            int psp_closeRoi;
            int psp_medianSize;
            double psp_roiMarginTop;
            double psp_roiMarginBottom;
            double psp_roiMarginLeft;
            double psp_roiMarginRight;

		public:
			OpticalOfflineAnalysisEngine(std::string& paramFilePath) : Engine<T>(paramFilePath), cam(NULL), _electrodes(), vec_menu()
            {
                
                /* create camera object */
                if( NULL == ( this->cam = MLARR::IO::RawFileCameraFactory<T>::create(
                     this->camType, this->dirPath, this->format, this->f_start, this->f_skip, this->f_stop ))){
                    throw "failed to create camera object";
                }
                
                /* load parameter file (JSON) */
                std::ifstream ifs(paramFilePath.c_str());
                if( !ifs.is_open() ){
                    throw "failed to open parameter file.";
                }
                char* buf = new char[1024];
                string str_json("");
                while (!ifs.eof()) {
                    ifs.getline(buf, 1024);
                    str_json += string( buf );
                }
                ifs.close();

                picojson::value val;
                std::string  err;
                picojson::parse(val, str_json.c_str(), str_json.c_str() + str_json.size(), &err);
                if( !err.empty() ){
                    throw "failed to parse json";
                }
                
                picojson::object& obj = val.get<picojson::object>();
                picojson::object& opt = obj["opticalOffline"].get<picojson::object>();
                
                picojson::array& menu = opt["menu"].get<picojson::array>();
                for( picojson::array::iterator it = menu.begin(); it != menu.end(); it++){
                    this->vec_menu.push_back( it->get<std::string>() );
                }
                
                picojson::array& elc = opt["electrode"].get<picojson::array>();
                for( picojson::array::iterator it = elc.begin(); it != elc.end(); it++){
                    picojson::object& e = it->get<picojson::object>();
                    _electrodes.push_back(MLARR::IO::Electrode(
                       atoi(e["id"].to_str().c_str()),
                       atoi(e["x"].to_str().c_str()),
                       atoi(e["y"].to_str().c_str())));
                }
                
                picojson::object& rnm = opt["revNorm"].get<picojson::object>();
                rnm_minDelta = atoi( rnm["minDelta"].to_str().c_str() );
                
                picojson::object& hbt = opt["hilbert"].get<picojson::object>();
                hbt_size = atoi( hbt["size"].to_str().c_str() );
                hbt_filSize = atoi( hbt["filterSize"].to_str().c_str() );
                hbt_minPeakDistance = atoi( hbt["minPeakDistance"].to_str().c_str());
                
                picojson::object& psp = opt["phaseSingularity"].get<picojson::object>();
                psp_openRoi = atoi( psp["openRoi"].to_str().c_str() );
                psp_openPS = atoi( psp["openPS"].to_str().c_str() );
                psp_closeRoi = atoi( psp["closeRoi"].to_str().c_str() );
                psp_medianSize = atoi( psp["medianFilterSize"].to_str().c_str());
                picojson::object& psp_roi = psp["roiMargin"].get<picojson::object>();
                psp_roiMarginTop = atof( psp_roi["top"].to_str().c_str() );
                psp_roiMarginBottom = atof( psp_roi["bottom"].to_str().c_str() );
                psp_roiMarginLeft = atof( psp_roi["left"].to_str().c_str() );
                psp_roiMarginRight = atof( psp_roi["right"].to_str().c_str() );
                
			};
            
			~OpticalOfflineAnalysisEngine(void){
                delete this->cam;
            };
	
			/* Reverse Normalization */
			void revNorm(void)
			{
				MaxImageAnalyzer<unsigned short> maxEng(0, *(this->cam));
				MinImageAnalyzer<unsigned short> minEng(USHRT_MAX, *(this->cam));
				OpticalImageAnalyzer<unsigned short> optEng(*(this->cam), maxEng, minEng, 20);

				Dumper<unsigned char> dump_roi( optEng.getRoi(), this->dstDir, fmt_raw_roi);
				Dumper<double> dump_opt( optEng, this->dstDir, fmt_raw_opt);
				
				Display<unsigned short> disp_cam("gray", *(this->cam), ( 1 << this->cam->bits )-1, 0, colMap_gray );
				Display<unsigned short> disp_max("max", maxEng, ( 1 << this->cam->bits )-1, 0, colMap_gray );
				Display<unsigned short> disp_min("min", minEng, ( 1 << this->cam->bits )-1, 0, colMap_gray );
				Display<double> disp_opt("optical", optEng, 1.0, 0.0, colMap_orange);

				while( stop != this->cam->state ){
					this->cam->capture();
					maxEng.execute();
					minEng.execute();
					disp_cam.show( this->cam->getTime(), white );
				}
				optEng.updateRange();
				dump_roi.dump(this->cam->f_tmp);
				
                disp_max.show();
				disp_min.show();
				disp_max.save( this->dstDir, fmt_jpg_max, this->cam->f_tmp);
				disp_min.save( this->dstDir, fmt_jpg_min, this->cam->f_tmp);

				this->cam->initialize();
				while( stop != this->cam->state ){
					this->cam->capture();
					optEng.execute();
					disp_opt.show( this->cam->getTime(), white );
					disp_opt.save( this->dstDir, fmt_jpg_opt, this->cam->f_tmp);
					dump_opt.dump( this->cam->f_tmp );
				}
				this->cam->initialize();
				
			};

			/* Phase analysis using hilbert transform.*/
			void hilbertPhase(void){
                
                using namespace std;
                using namespace MLARR::Analyzer;
                using namespace MLARR::Basic;

				Coeffs coeff;

				RawFileCamera<double> camOpt(
					this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
					this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip, this->cam->f_stop );
				RawFileCamera<unsigned char> camRoi(
					this->cam->width, this->cam->height, std::numeric_limits<char>::digits, this->cam->fps,
					this->dstDir, fmt_raw_roi, 1,1,1);
                
                std::vector<ImageShrinker<double>*> vec_optComp;
                std::vector<ImageShrinker<unsigned char>*> vec_roiComp;
                
                int tmpWidth = camOpt.width;
                while( tmpWidth > hbt_size ){
                    if( vec_optComp.size() ){
                        vec_optComp.push_back( new ImageShrinker<double>( dynamic_cast<Image<double>&>(*vec_optComp[vec_optComp.size() - 1 ])));
                        vec_roiComp.push_back( new ImageShrinker<unsigned char>( dynamic_cast<Image<unsigned char>&>(*vec_roiComp[vec_roiComp.size() - 1 ])));
                    }else{
                        vec_optComp.push_back( new ImageShrinker<double>( camOpt ) );
                        vec_roiComp.push_back( new ImageShrinker<unsigned char>( camRoi ) );
                    }
                    tmpWidth = vec_optComp.back()->width;
                }
                
				HilbertAnalyzer<double> hilbertEng(
					*vec_optComp.back(), hbt_minPeakDistance, hbt_filSize,
					this->cam->f_start,this->cam->f_skip,this->cam->f_stop);

				Dumper<double> dump_hilbert( hilbertEng, this->dstDir, fmt_raw_hbt);
				Dumper<unsigned char> dump_roi_hbt( *vec_roiComp.back(), this->dstDir, fmt_raw_roh);

				Display<double> disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
				Display<double> disp_hilbert("hilbert", hilbertEng, M_PI, -M_PI, colMap_hsv);

				/* ROI setting */
				camRoi.capture();
                for( vector< ImageShrinker<unsigned char>*>::iterator it = vec_roiComp.begin(); it != vec_roiComp.end(); it++ ){
                    (*it)->execute();
                }
				hilbertEng.setRoi( *vec_roiComp.back() );
				dump_roi_hbt.dump( camRoi.f_tmp );

				camOpt.initialize();
				while( stop != camOpt.state && error != camOpt.state ){
					camOpt.capture();
                    for( std::vector<ImageShrinker<double>*>::iterator it = vec_optComp.begin(); it != vec_optComp.end(); it++ ){
                        (*it)->execute();
                    }
					hilbertEng.updateSrc();
					disp_opt.show( camOpt.getTime(), white );
				}
				camOpt.initialize();
				while( stop != camOpt.state && error != camOpt.state ){
					camOpt.capture();
					hilbertEng.execute();
					
					disp_opt.show( camOpt.getTime(), white );
					disp_hilbert.show( camOpt.getTime(), red );

					disp_hilbert.save( this->dstDir, fmt_jpg_hbt, camOpt.f_tmp);
					dump_hilbert.dump( camOpt.f_tmp);
				}
				camOpt.initialize();
                
                /* delete newed images */
                for( vector< ImageShrinker<unsigned char>*>::iterator it = vec_roiComp.begin(); it != vec_roiComp.end(); it++ ){
                    delete *it;
                }
                for( vector< ImageShrinker<double>*>::iterator it = vec_optComp.begin(); it != vec_optComp.end(); it++ ){
                    delete *it;
                }

			};

			void monitorElecPhase(void){

				using namespace std;
				using namespace MLARR::IO;
				using namespace MLARR::Analyzer;

				char buf[255];
				sprintf( buf, fmt_log_phs.c_str(), this->dstDir.c_str() );
				ofstream ofs(buf);
				if( !ofs ) throw "failed to open file with format " + fmt_log_phs;
                int hbt_compRate = this->cam->width / hbt_size + ( this->cam->width % hbt_size == 0 ? 0 : 1);


				RawFileCamera<double> camHbt(
					hbt_size, hbt_size, std::numeric_limits<double>::digits, this->cam->fps,
					this->dstDir, fmt_raw_hbt, this->cam->f_start, this->cam->f_skip, this->cam->f_stop );
				
				Display<double> dispHbtFil("hilbert(filtered)", camHbt, M_PI, -M_PI, colMap_hsv);
				
				camHbt.initialize();
				while( stop != camHbt.state && error != camHbt.state ){

					camHbt.capture();
					
					dispHbtFil.show( camHbt.getTime(), red );

					ofs << camHbt.getTime() << " ";
					vector<MLARR::IO::Electrode>::iterator it;
					for( it = _electrodes.begin(); it != _electrodes.end(); it++){
						int x  = it->getX() / hbt_compRate;
						int y  = it->getY() / hbt_compRate;
                        int i = it->getID();
						MLARR::Analyzer::ImageCropper<double> crop(  dynamic_cast<Image<double>&>(camHbt), x-2, y-2, 5, 5 );
						MLARR::Analyzer::MedianFilter<double> hbtFil(  dynamic_cast<Image<double>&>(crop), 5 );
						crop.execute();
						hbtFil.execute();
						double phase = *( hbtFil.getRef( 2, 2) );
						ofs << i << " " << phase << " ";
					}
					ofs << endl;

				}
				camHbt.initialize();

				ofs.close();

			};

			/* Phase Singurality Analysis */
			void phaseSingularityAnalysis(void)
			{
                using namespace MLARR::Analyzer;
                using namespace MLARR::Basic;
				using namespace std;
                
                int hbt_compRate = this->cam->width / hbt_size + ( this->cam->width % hbt_size == 0 ? 0 : 1);

                RawFileCamera<double> camOpt(
                     this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                     this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );
				RawFileCamera<double> camPhase(
					this->cam->width/hbt_compRate, this->cam->height/hbt_compRate, std::numeric_limits<double>::digits, this->cam->fps,
					this->dstDir, fmt_raw_hbt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );
				RawFileCamera<unsigned char> camRoi(
                    this->cam->width/hbt_compRate, this->cam->height/hbt_compRate, std::numeric_limits<char>::digits, this->cam->fps,
                    this->dstDir, fmt_raw_roh, 1,1,1);
                
                MLARR::Analyzer::MorphImage<unsigned char> preOpenRoi(camRoi, psp_openRoi);
                MLARR::Analyzer::MorphImage<unsigned char> closeRoi(preOpenRoi, -1*psp_closeRoi);
                MLARR::Analyzer::MedianFilter<double> imgMed( dynamic_cast<Image<double>&>(camPhase), psp_medianSize);
                MLARR::Analyzer::PhaseSpacialFilter<double> imgFil( dynamic_cast<Image<double>&>(imgMed), 5,5, MLARR::Analyzer::coefficients.vec_gaussian_5x5 );
                MLARR::Analyzer::AdjPhaseSingularityAnalyzer<double> imgPS(dynamic_cast<Image<double>&>(imgFil));
                MLARR::Analyzer::PyramidDetector< double, ImageShrinker<double>, AdjPhaseSingularityAnalyzer<double> > imgPyrPS( imgFil, &imgPS, 3 );
                
                
                MLARR::Analyzer::MorphImage<unsigned char> imgOpenPS( imgPyrPS, psp_openPS );
                MLARR::Analyzer::LabelImage imgLabel(dynamic_cast<Image<unsigned char>&>(imgOpenPS));

                
                /* set roi */
                camRoi.capture();
                preOpenRoi.execute();
                closeRoi.execute();
                for( int j = 0; j < closeRoi.height; j++ ){
                    if( j <= closeRoi.height * psp_roiMarginTop || j >= closeRoi.height * (1 - psp_roiMarginBottom)) {
                        for( int i = 0; i < closeRoi.width; i++ ){
                            closeRoi.setValue( i, j, 0);
                        }
                    }
                }
                for( int i = 0; i < closeRoi.width; i++ ){
                    if( i <= closeRoi.height * psp_roiMarginLeft || i >= closeRoi.height * (1 - psp_roiMarginRight)) {
                        for( int j = 0; j < closeRoi.height; j++ ){
                            closeRoi.setValue( i, j, 0);
                        }
                    }
                }
                imgPyrPS.setRoi( dynamic_cast<const Image<unsigned char>&>(closeRoi) );
                

                Display<double> disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
                Display<double> dispFil  ("Filtered Phase Map", imgFil, M_PI, -M_PI, colMap_hsv);
                Display<unsigned char> dispPS   ("Phase Singurality Image", imgOpenPS, 1, 0, colMap_gray);
                
                std::vector< IO::Display<unsigned char>* > vec_disp;
                for( int i = 0; i < imgPyrPS.vec_analyzer.size(); i++){
                    char buf[128];
                    sprintf(buf, "Pyramid Display %d", i);
                    MLARR::Analyzer::AdjPhaseSingularityAnalyzer<double>* ptr = imgPyrPS.vec_analyzer[i];
                    vec_disp.push_back(new IO::Display<unsigned char>( std::string(buf), *ptr, 1, 0, IO::colMap_gray, ptr->width, ptr->height ));
                }
                
                char buf[255];
				sprintf( buf, fmt_log_psp.c_str(), this->dstDir.c_str() );
				ofstream ofs(buf);
				if( !ofs ) throw "failed to open file with format " + fmt_log_psp;
                
                camPhase.initialize();
                while( stop != camPhase.state && error != camPhase.state ){

                    camPhase.capture();
                    camOpt.capture();
                    imgFil.execute();
                    imgMed.execute();
                    imgPyrPS.execute();
                    imgOpenPS.execute();
                    imgLabel.execute();

                    ofs << camPhase.getTime() << "," ;
                    
                    std::vector<MLARR::Basic::Point<double> >::iterator it = imgLabel.vec_ps.begin();
                    while( it != imgLabel.vec_ps.end() ){
                        disp_opt.drawRect(
                            static_cast<int>(it->getX() - 2)*hbt_compRate,
                            static_cast<int>(it->getY() - 2)*hbt_compRate,
                            static_cast<int>(it->getX() + 2)*hbt_compRate,
                            static_cast<int>(it->getY() + 2)*hbt_compRate,
                                         white);
                        dispFil.drawRect(
                                          static_cast<int>(it->getX() - 2),
                                          static_cast<int>(it->getY() - 2),
                                          static_cast<int>(it->getX() + 2),
                                          static_cast<int>(it->getY() + 2),
                                          white);
                        ofs << it->getX()*hbt_compRate << "," << it->getY()*hbt_compRate << ",";
                        it++;
                    }
                    
                    ofs << std::endl;
                    
                    disp_opt.show( camPhase.getTime(), red);
                    dispFil.show( camPhase.getTime(), red);
                    dispPS.show( camPhase.getTime(), red);
                    for( int i = 0; i < vec_disp.size(); i++){
                        vec_disp[i]->show();
                    }
                    
                    
                    disp_opt.save(this->dstDir, fmt_jpg_psp, camPhase.getTime());
                    dispFil.save(this->dstDir, fmt_jpg_hbf, camPhase.getTime());
                }
                
                for( int i = 0; i < vec_disp.size(); i++){
                    delete vec_disp[i];
                }
                
                ofs.close();

			};
            
            void SimplePhaseAnalysis(){
                
                RawFileCamera<double> optCam(
                     this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                     this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip, this->cam->f_stop );
				
                ImageShrinker<double> optSh1( optCam );
				ImageShrinker<double> optSh2( dynamic_cast<Image<double>&>(optSh1) );
				ImageShrinker<double> optSh3( dynamic_cast<Image<double>&>(optSh2) );
				ImageShrinker<double> optSh4( dynamic_cast<Image<double>&>(optSh3) );
				SimplePhaseAnalyzer<double> spEng1( optSh1 , MLARR::Analyzer::coefficients.vec_spFIR );
				SimplePhaseAnalyzer<double> spEng2( optSh2 , MLARR::Analyzer::coefficients.vec_spFIR );
				SimplePhaseAnalyzer<double> spEng3( optSh3 , MLARR::Analyzer::coefficients.vec_spFIR );
				SimplePhaseAnalyzer<double> spEng4( optSh4 , MLARR::Analyzer::coefficients.vec_spFIR );
				SimplePhaseSingularityAnalyzer<double> spsEng1( spEng1 );
				SimplePhaseSingularityAnalyzer<double> spsEng2( spEng2 );
				SimplePhaseSingularityAnalyzer<double> spsEng3( spEng3 );
				SimplePhaseSingularityAnalyzer<double> spsEng4( spEng4 );
				
				ImageDoubler<char> spsDouble4( spsEng4 );
				BinaryAnd<char> sps43( spsEng3, spsDouble4 );
				ImageDoubler<char> spsDouble43( sps43 );
				BinaryAnd<char> sps432( spsEng2, spsDouble43 );
				ImageDoubler<char> spsDouble432( sps432 );
				BinaryAnd<char> sps4321( spsEng1, spsDouble432 );
				
				Display<double> disp_opt("optical", optCam, 1.0, 0.0, colMap_orange);
				Display<double> disp_opt_sh1("1", optSh1, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
				Display<double> disp_opt_sh2("2", optSh2, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
				Display<double> disp_opt_sh3("3", optSh3, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
				Display<double> disp_opt_sh4("4", optSh4, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
				Display<char> disp_sg1( "sg1", spEng1.img_sign, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sg2( "sg2", spEng2.img_sign, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sg3( "sg3", spEng3.img_sign, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sg4( "sg4", spEng4.img_sign, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sp1( "sp1", spEng1, 5, 0, colMap_orange, this->cam->width, this->cam->height );
				Display<char> disp_sp2( "sp2", spEng2, 5, 0, colMap_orange, this->cam->width, this->cam->height );
				Display<char> disp_sp3( "sp3", spEng3, 5, 0, colMap_orange, this->cam->width, this->cam->height );
				Display<char> disp_sp4( "sp4", spEng4, 5, 0, colMap_orange, this->cam->width, this->cam->height );
				Display<char> disp_sps1( "sps1", spsEng1, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sps2( "sps2", spsEng2, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sps3( "sps3", spsEng3, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sps4( "sps4", spsEng4, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sps43( "sps43", sps43, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sps432( "sps432", sps432, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				Display<char> disp_sps4321( "sps4321", sps4321, 1, 0, colMap_gray, this->cam->width, this->cam->height );
				
				while( stop != optCam.state && error != optCam.state ){
					this->cam->capture();
					optSh1.execute();
					optSh2.execute();
					optSh3.execute();
					optSh4.execute();
					spEng4.execute();
					spsEng4.execute();
					spEng3.execute();
					spsEng3.execute();
					spEng2.execute();
					spsEng2.execute();
					spEng1.execute();
					spsEng1.execute();
					spsDouble4.execute();
					sps43.execute();
					spsDouble43.execute();
					sps432.execute();
					spsDouble432.execute();
					sps4321.execute();
					disp_opt.show(optCam.getTime(), white);
					disp_opt_sh1.show(optCam.getTime(), white);
					disp_opt_sh2.show(optCam.getTime(), white);
					disp_opt_sh3.show(optCam.getTime(), white);
					disp_opt_sh4.show(optCam.getTime(), white);
					disp_sg1.show(optCam.getTime(), red);
					disp_sg2.show(optCam.getTime(), red);
					disp_sg3.show(optCam.getTime(), red);
					disp_sg4.show(optCam.getTime(), red);
					disp_sp1.show(optCam.getTime(), white);
					disp_sp2.show(optCam.getTime(), white);
					disp_sp3.show(optCam.getTime(), white);
					disp_sp4.show(optCam.getTime(), white);
					disp_sps1.show(optCam.getTime(), red);
					disp_sps2.show(optCam.getTime(), red);
					disp_sps3.show(optCam.getTime(), red);
					disp_sps4.show(optCam.getTime(), red);
					disp_sps43.show(optCam.getTime(), red);
					disp_sps432.show(optCam.getTime(), red);
					disp_sps4321.show(optCam.getTime(), red);
					if( 'c' == cv::waitKey(5) ) break;
				}
                
			};
            
            void execute(void){

                if( this->vec_menu.size() == 1 && vec_menu[0] == "full" ){
                    vec_menu.push_back("revnorm");
                    vec_menu.push_back("hilbert");
                    vec_menu.push_back("monitor");
                    vec_menu.push_back("psdetect");
                }
                
                for( std::vector<std::string>::iterator it = vec_menu.begin(); it != vec_menu.end(); it++ ){
                    if(*it == "revnorm")
                        revNorm();
                    if(*it == "hilbert")
                        hilbertPhase();
                    if(*it == "monitor")
                        monitorElecPhase();
                    if(*it == "psdetect")
                        phaseSingularityAnalysis();
                }
                
            };
            
		};
	}
}