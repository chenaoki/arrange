#pragma once

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
#include "MovieAnalyzer.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"

using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;

namespace MLARR{
	namespace Engine{

		const std::string fmt_raw_roi("%s/raw/roi.raw");
		const std::string fmt_raw_roi_q("%s/raw/roi_q.raw");
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
		const std::string fmt_log_phs("%s/log/phs.log");

		template <class CAM>
		class EngineOffLine{

		private:
			CAM& cam;
			std::vector<MLARR::IO::Electrode>& electrodes;
			const std::string dstDir;

		public:
			EngineOffLine( CAM& _cam, std::vector<MLARR::IO::Electrode>& _electrodes, const std::string _dstDir)
				: dstDir( _dstDir ), cam(_cam), electrodes(_electrodes) {
					std::string temp;
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/raw";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/raw/max";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/raw/min";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/raw/opt";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/raw/hbt";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg/max";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg/min";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg/opt";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg/hbt";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg/hbf";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/jpg/psh";
					mkdir( temp.c_str() , 0777);
					temp = _dstDir + "/log";
					mkdir( temp.c_str() , 0777);

			};
			~EngineOffLine(void){};
	
			/* Reverse Normalization */
			void revNorm(void)
			{
				MaxImageAnalyzer<unsigned short> maxEng(0, cam);
				MinImageAnalyzer<unsigned short> minEng(USHRT_MAX, cam);
				OpticalImageAnalyzer<unsigned short> optEng(cam, maxEng, minEng, 20);

				Dumper<unsigned char> dump_roi( optEng.getRoi(), dstDir, fmt_raw_roi);
				Dumper<double> dump_opt( optEng, dstDir, fmt_raw_opt);
				
				Display<unsigned short> disp_cam("gray", cam, ( 1 << cam.bits )-1, 0, colMap_gray );
				Display<unsigned short> disp_max("max", maxEng, ( 1 << cam.bits )-1, 0, colMap_gray );
				Display<unsigned short> disp_min("min", minEng, ( 1 << cam.bits )-1, 0, colMap_gray );
				Display<double> disp_opt("optical", optEng, 1.0, 0.0, colMap_orange);

				while( stop != cam.state ){
					cam.capture();
					maxEng.execute();
					minEng.execute();
					disp_cam.show();
				}
				optEng.updateRange();

				dump_roi.dump(cam.f_tmp);

				disp_max.show();
				disp_min.show();
				disp_max.save(dstDir, fmt_jpg_max, cam.f_tmp);
				disp_min.save(dstDir, fmt_jpg_min, cam.f_tmp);

				cam.initialize();
				while( stop != cam.state ){
					cam.capture();
					optEng.execute();
					disp_opt.show( cam.getTime(), white );
					disp_opt.save( dstDir, fmt_jpg_opt, cam.f_tmp);
					dump_opt.dump( cam.f_tmp );
				}
				cam.initialize();
				
			};

			/* Phase analysis using hilbert transform.*/
			void hilbertPhase(void){

				const int minPeakDistance = 20;
				const int filterSize = 10;
				Coeffs coeff;

				RawFileCamera<double> optCam( 
					cam.width, cam.height, std::numeric_limits<double>::digits, cam.fps, 
					dstDir, fmt_raw_opt, cam.f_start,cam.f_skip,cam.f_stop );
				RawFileCamera<unsigned char> camRoi(
					cam.width, cam.height, std::numeric_limits<char>::digits, cam.fps, 
					dstDir, fmt_raw_roi, 1,1,1);

				MLARR::Analyzer::ImageShrinker<double> optHalf( optCam );
				MLARR::Analyzer::ImageShrinker<double> optQuarter( dynamic_cast<Image<double>&>(optHalf) );
				MLARR::Analyzer::HilbertAnalyzer<double> hilbertEng(
					optQuarter, minPeakDistance, filterSize, 
					cam.f_start,cam.f_skip,cam.f_stop);
				MLARR::Analyzer::ImageThinOut<unsigned char> roiHalf( camRoi );
				MLARR::Analyzer::ImageThinOut<unsigned char> roiQuarter( dynamic_cast<Image<unsigned char>&>(roiHalf) );

				Dumper<double> dump_hilbert( hilbertEng, dstDir, fmt_raw_hbt);
				Dumper<unsigned char> dump_roi_quarter( roiQuarter, dstDir, fmt_raw_roi_q);

				Display<double> disp_opt("optical", optCam, 1.0, 0.0, colMap_orange);
				Display<double> disp_hilbert("hilbert", hilbertEng, M_PI, -M_PI, colMap_hsv);

				/* ROI setting */
				camRoi.capture();
				roiHalf.execute();
				roiQuarter.execute();
				hilbertEng.setRoi( roiQuarter );
				dump_roi_quarter.dump( camRoi.f_tmp );

				optCam.initialize();
				while( stop != optCam.state && error != optCam.state ){
					optCam.capture();
					optHalf.execute();
					optQuarter.execute();
					hilbertEng.updateSrc();
					
					disp_opt.show( optCam.getTime(), white );
				}
				optCam.initialize();
				while( stop != optCam.state && error != optCam.state ){
					optCam.capture();
					hilbertEng.execute();
					
					disp_opt.show( optCam.getTime(), white );
					disp_hilbert.show( optCam.getTime(), red );

					disp_hilbert.save( dstDir, fmt_jpg_hbt, optCam.f_tmp);
					dump_hilbert.dump( optCam.f_tmp);
				}
				optCam.initialize();

			};

			void monitorElecPhase(void){

				using namespace std;
				using namespace MLARR::IO;
				using namespace MLARR::Analyzer;

				char buf[255];
				sprintf( buf, fmt_log_phs.c_str(), dstDir.c_str() );
				ofstream ofs(buf);
				if( !ofs ) return;

				RawFileCamera<double> camHbt(
					cam.width/4, cam.height/4, std::numeric_limits<double>::digits, cam.fps, 
					dstDir, fmt_raw_hbt, cam.f_start,cam.f_skip,cam.f_stop );
				
				Display<double> dispHbtFil("hilbert(filtered)", camHbt, M_PI, -M_PI, colMap_hsv);
				
				camHbt.initialize();
				while( stop != camHbt.state && error != camHbt.state ){

					camHbt.capture();
					
					dispHbtFil.show( camHbt.getTime(), red );

					ofs << camHbt.getTime() << " ";
					vector<MLARR::IO::Electrode>::iterator it;
					for( it = electrodes.begin(); it != electrodes.end(); it++){
						int x  = it->getX();
						int y  = it->getY();
                        int i = it->getID();
						MLARR::Analyzer::ImageCropper<double> crop(  dynamic_cast<Image<double>&>(camHbt), x/4-2, y/4-2, 5, 5 );
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
				using namespace std;
                
                RawFileCamera<double> camOpt(
                     cam.width, cam.height, std::numeric_limits<double>::digits, cam.fps,
                     dstDir, fmt_raw_opt, cam.f_start,cam.f_skip,cam.f_stop );
				RawFileCamera<double> camPhase(
					cam.width/4, cam.height/4, std::numeric_limits<double>::digits, cam.fps,
					dstDir, fmt_raw_hbt, cam.f_start,cam.f_skip,cam.f_stop );
				RawFileCamera<unsigned char> camRoi(
                    cam.width/4, cam.height/4, std::numeric_limits<char>::digits, cam.fps,
                    dstDir, fmt_raw_roi_q, 1,1,1);
                
                MLARR::Analyzer::MorphImage<unsigned char> preOpenRoi(camRoi, 3);
                MLARR::Analyzer::MorphImage<unsigned char> closeRoi(preOpenRoi, -13);
                MLARR::Analyzer::MedianFilter<double> imgMed( dynamic_cast<Image<double>&>(camPhase), 5);
                MLARR::Analyzer::PhaseSpacialFilter<double> imgFil( dynamic_cast<Image<double>&>(imgMed), 5,5, MLARR::Analyzer::coefficients.vec_gaussian_5x5 );
                MLARR::Analyzer::DivPhaseSingularityAnalyzer<double> imgPS(dynamic_cast<Image<double>&>(imgFil), 3, 0.125);
                MLARR::Analyzer::LabelImage imgLabel(dynamic_cast<Image<unsigned char>&>(imgPS));

                camRoi.capture();
                preOpenRoi.execute();
                closeRoi.execute();
                
                /* set margin for PS detection area */
                for( int m = 0; m < 5; m++ ){
                    for( int i = 0; i < closeRoi.width; i++ ){
                        closeRoi.setValue( i, m, 0);
                        closeRoi.setValue( i, closeRoi.height - 1 - m, 0);
                    }
                    for( int j = 0; j < closeRoi.height; j++ ){
                        closeRoi.setValue( m, j, 0);
                        closeRoi.setValue( closeRoi.width - 1 - m, j, 0);
                    }
                }
                for( int j = 0; j < closeRoi.height; j++ ){
                    if( j < closeRoi.height / 2 ) {
                        for( int i = 0; i < closeRoi.width; i++ ){
                            closeRoi.setValue( i, j, 0);
                        }
                    }
                }
                
                
                imgPS.setRoi( dynamic_cast<const Image<unsigned char>&>(closeRoi) );
                

                /*
                Display<unsigned char> dispCloseRoi("closed roi", closeRoi, 1, 0, colMap_gray);
                Display<double> dispPhase("Phase Map", camPhase, M_PI, -M_PI, colMap_hsv);
                Display<double> dispMed  ("Median Phase Map", imgMed, M_PI, -M_PI, colMap_hsv);
                Display<double> dispFil  ("Filtered Phase Map", imgFil, M_PI, -M_PI, colMap_hsv);
                Display<unsigned char> dispPS   ("Phase Singurality Image", imgPS, 1, 0, colMap_gray);
                Display<unsigned char> dispLabel("Phase Singurality Label", imgLabel, 4, 0, colMap_gray);
                */
                Display<double> disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);

                
                camPhase.initialize();
                while( stop != camPhase.state && error != camPhase.state ){

                    camPhase.capture();
                    camOpt.capture();
                    imgFil.execute();
                    imgMed.execute();
                    imgPS.execute();
                    imgLabel.execute();
                    
                    std::vector<MLARR::Basic::Point<double>>::iterator it = imgLabel.vec_ps.begin();
                    while( it != imgLabel.vec_ps.end() ){
                        disp_opt.drawRect(
                            static_cast<int>(it->getX() - 2)*4,
                            static_cast<int>(it->getY() - 2)*4,
                            static_cast<int>(it->getX() + 2)*4,
                            static_cast<int>(it->getY() + 2)*4,
                                         white);
                        it++;
                    }
                    
                    /*
                    dispPhase.show( camPhase.getTime(), red);
                    dispFil.show( camPhase.getTime(), red);
                    dispMed.show( camPhase.getTime(), red );
                    dispPS.show( camPhase.getTime(), red);
                    dispLabel.show( camPhase.getTime(), red);
                    dispMed.save(dstDir, fmt_jpg_hbf, camPhase.getTime());
                    dispPS.save(dstDir, fmt_jpg_psh, camPhase.getTime());
                    */
                    disp_opt.show( camPhase.getTime(), red);
                    
                    /*
                    dispDiffX.show(camPhase.getTime(), red);
                    dispDiffY.show(camPhase.getTime(), red);
                    dispCurlX.show(camPhase.getTime(), red);
                    dispCurlY.show(camPhase.getTime(), red);
                    dumpCurlX.dumpText(camPhase.getTime());
                    dumpCurlY.dumpText(camPhase.getTime());
                    dumpCurlA.dumpText(camPhase.getTime());
                    */
                }

			};


			/* Simple Phase Analysis */
			void simplePhase(void)
			{
				Coeffs coeff;
				RawFileCamera<double> optCam(
					cam.width, cam.height, std::numeric_limits<double>::digits, 
					cam.fps, dstDir, fmt_raw_opt, 
					cam.f_start,cam.f_skip,cam.f_stop);

				ImageShrinker<double> optSh1( optCam );
				ImageShrinker<double> optSh2( dynamic_cast<Image<double>&>(optSh1) );
				ImageShrinker<double> optSh3( dynamic_cast<Image<double>&>(optSh2) );
				ImageShrinker<double> optSh4( dynamic_cast<Image<double>&>(optSh3) );
				SimplePhaseAnalyzer<double> spEng1( optSh1 , coeff.vec_spFIR );
				SimplePhaseAnalyzer<double> spEng2( optSh2 , coeff.vec_spFIR );
				SimplePhaseAnalyzer<double> spEng3( optSh3 , coeff.vec_spFIR );
				SimplePhaseAnalyzer<double> spEng4( optSh4 , coeff.vec_spFIR );
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
				Display<double> disp_opt_sh1("1", optSh1, 1.0, 0.0, colMap_orange, cam.width, cam.height );
				Display<double> disp_opt_sh2("2", optSh2, 1.0, 0.0, colMap_orange, cam.width, cam.height );
				Display<double> disp_opt_sh3("3", optSh3, 1.0, 0.0, colMap_orange, cam.width, cam.height );
				Display<double> disp_opt_sh4("4", optSh4, 1.0, 0.0, colMap_orange, cam.width, cam.height );
				Display<char> disp_sg1( "sg1", spEng1.img_sign, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sg2( "sg2", spEng2.img_sign, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sg3( "sg3", spEng3.img_sign, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sg4( "sg4", spEng4.img_sign, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sp1( "sp1", spEng1, 5, 0, colMap_orange, cam.width, cam.height ); 
				Display<char> disp_sp2( "sp2", spEng2, 5, 0, colMap_orange, cam.width, cam.height ); 
				Display<char> disp_sp3( "sp3", spEng3, 5, 0, colMap_orange, cam.width, cam.height ); 
				Display<char> disp_sp4( "sp4", spEng4, 5, 0, colMap_orange, cam.width, cam.height ); 
				Display<char> disp_sps1( "sps1", spsEng1, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sps2( "sps2", spsEng2, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sps3( "sps3", spsEng3, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sps4( "sps4", spsEng4, 1, 0, colMap_gray, cam.width, cam.height );
				Display<char> disp_sps43( "sps43", sps43, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sps432( "sps432", sps432, 1, 0, colMap_gray, cam.width, cam.height ); 
				Display<char> disp_sps4321( "sps4321", sps4321, 1, 0, colMap_gray, cam.width, cam.height ); 
				
				while( stop != optCam.state && error != optCam.state ){
					optCam.capture();
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

		};
	}
}