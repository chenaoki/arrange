//
//  OpticalOfflineAnalysisEngine.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/09/04.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_OpticalOfflineAnalysisEngine_hpp
#define Arrange_OpticalOfflineAnalysisEngine_hpp

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
#include "Hilbert.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "Optical.h"
#include "Engine.h"
#include "CameraFactory.hpp"

using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;
using namespace std;

namespace MLARR{
	namespace Engine{
        
        const std::string fmt_raw_roi("%s/raw/roi.raw");
		const std::string fmt_raw_roh("%s/raw/roh.raw");
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
		const std::string fmt_jpg_ecg("%s/jpg/ecg/ecg_%05d.jpg");
		const std::string fmt_jpg_iso("%s/jpg/iso/iso_%05d.jpg");
		const std::string fmt_log_phs("%s/log/phs.csv");
        const std::string fmt_log_psp("%s/log/psp.csv");
        const std::string fmt_log_act("%s/log/act.csv");


        template<typename T>
        class OpticalOfflineAnalysisEngine : public Engine{
            
        private:
            RawFileCamera<T>* cam;
            std::vector<std::string> vec_menu;
            std::vector<MLARR::IO::Electrode> _electrodes;
            T rnm_minDelta;
            int hbt_size;
            int hbt_minPeakDistance;
            int hbt_filSize;
            int hbt_elecCompPix;
            std::string hbt_elecImagePath;
            int psp_closeRoi;
            int psp_medianSize;
            int psp_adjacentSize;
            double psp_phaseFrontMean;
            double psp_phaseFrontRange;
            int psp_isoMax;
            int psp_isoInterval;
            double psp_roiMarginTop;
            double psp_roiMarginBottom;
            double psp_roiMarginLeft;
            double psp_roiMarginRight;
            std::string camType;
            
        public:
            OpticalOfflineAnalysisEngine(std::string& paramFilePath) : Engine(paramFilePath), cam(NULL), _electrodes(), vec_menu()
            {
                
                /* make output directories */
                std::string temp = this->dstDir + "/raw";
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
                temp = this->dstDir + "/jpg/ecg";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/iso";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/log";
                mkdir( temp.c_str() , 0777);
                
                /* load parameter file (JSON) */
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath);
                camType = obj["camType"].get<std::string>();
                
                picojson::object& opt = obj["opticalOffline"].get<picojson::object>();
                picojson::array& menu = opt["menu"].get<picojson::array>();
                for( picojson::array::iterator it = menu.begin(); it != menu.end(); it++){
                    this->vec_menu.push_back( it->get<std::string>() );
                }
                
                picojson::object& rnm = opt["revnorm"].get<picojson::object>();
                rnm_minDelta = atoi( rnm["minDelta"].to_str().c_str() );
                
                picojson::object& hbt = opt["hilbert"].get<picojson::object>();
                hbt_size = atoi( hbt["size"].to_str().c_str() );
                hbt_filSize = atoi( hbt["filterSize"].to_str().c_str() );
                hbt_minPeakDistance = atoi( hbt["minPeakDistance"].to_str().c_str());
                hbt_elecCompPix = atoi( hbt["elecCompPix"].to_str().c_str());
                hbt_elecImagePath = hbt["elecImagePath"].get<std::string>();
                
                picojson::object& psp = opt["psdetect"].get<picojson::object>();
                psp_closeRoi = atoi( psp["closeRoi"].to_str().c_str() );
                psp_medianSize = atoi( psp["medianFilterSize"].to_str().c_str());
                psp_adjacentSize = atoi( psp["adjacentSize"].to_str().c_str());
                psp_phaseFrontMean = atof( psp["phaseFrontMean"].to_str().c_str() );
                psp_phaseFrontRange = atof( psp["phaseFrontRange"].to_str().c_str() );
                picojson::object& psp_iso = psp["isochrone"].get<picojson::object>();
                psp_isoMax = atoi( psp_iso["max"].to_str().c_str());
                psp_isoInterval = atoi( psp_iso["interval"].to_str().c_str());
                picojson::object& psp_roi = psp["roiMargin"].get<picojson::object>();
                psp_roiMarginTop = atof( psp_roi["top"].to_str().c_str() );
                psp_roiMarginBottom = atof( psp_roi["bottom"].to_str().c_str() );
                psp_roiMarginLeft = atof( psp_roi["left"].to_str().c_str() );
                psp_roiMarginRight = atof( psp_roi["right"].to_str().c_str() );
                
                
                /* create camera object */
                if( NULL == ( this->cam = static_cast<RawFileCamera<T>*>( MLARR::IO::CameraFactory::create(this->camType, paramFilePath)))){
                    throw "failed to create camera object";
                }

                
            };
            
            ~OpticalOfflineAnalysisEngine(void){
                delete this->cam;
            };
            
            /* Reverse Normalization */
            void revNorm(void)
            {
                MaxImageAnalyzer<T> maxEng( 0, *(this->cam));
                MinImageAnalyzer<T> minEng( ( 1 << this->cam->bits ) - 1, *(this->cam));
                OpticalImageAnalyzer<T> optEng(*(this->cam), maxEng, minEng, rnm_minDelta);
                
                Dumper<unsigned char> dump_roi( optEng.getRoi(), this->dstDir, fmt_raw_roi);
                Dumper<double> dump_opt( optEng, this->dstDir, fmt_raw_opt);
                
                Display<T> disp_cam("gray", *(this->cam), ( 1 << this->cam->bits )-1, 0, colMap_gray );
                Display<T> disp_max("max", maxEng, ( 1 << this->cam->bits )-1, 0, colMap_gray );
                Display<T> disp_min("min", minEng, ( 1 << this->cam->bits )-1, 0, colMap_gray );
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
                
                /* Camera */
                RawFileCamera<double> camOpt(
                    this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                    this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip, this->cam->f_stop );
                RawFileCamera<unsigned char> camRoi(
                    this->cam->width, this->cam->height, std::numeric_limits<char>::digits, this->cam->fps,
                    this->dstDir, fmt_raw_roi, 1,1,1);
                
                /* Analyzer */
                std::vector<ImageShrinker<double>*> vec_optComp;
                std::vector<ImageShrinker<unsigned char>*> vec_roiComp;
                Image<unsigned char> *imgROI = NULL;
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
                Image<double>* imgOpt = NULL;
                if( !vec_optComp.size() ){
                    imgOpt = static_cast< Image<double>* >(&camOpt);
                }else{
                    imgOpt = static_cast< Image<double>* >(vec_optComp.back());
                }
                HilbertAnalyzer<double> hilbertEng(
                   *imgOpt, hbt_minPeakDistance, hbt_filSize,
                   this->cam->f_start,this->cam->f_skip,this->cam->f_stop);
                ElectrodePhaseComplement imgComp( hilbertEng, hbt_elecImagePath, hbt_elecCompPix);
                
                /* Dumper */
                Dumper<double> dump_hilbert( hilbertEng, this->dstDir, fmt_raw_hbt);
                
                /* Display */
                Display<double> disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
                Display<double> disp_hilbert("hilbert", imgComp, M_PI, -M_PI, colMap_hsv);
                
                /* ROI setting */
                camRoi.capture();
                if( vec_roiComp.size()){
                    for( vector< ImageShrinker<unsigned char>*>::iterator it = vec_roiComp.begin(); it != vec_roiComp.end(); it++ ){
                        (*it)->execute();
                    }
                    imgROI = vec_roiComp.back();
                }else{
                    imgROI = &camRoi;
                }
                hilbertEng.setRoi( *imgROI );
                imgComp.setRoi( *imgROI );
                Dumper<unsigned char> dump_roi_hbt( *imgROI, this->dstDir, fmt_raw_roh);
                dump_roi_hbt.dump( camRoi.f_tmp );
                Display<unsigned char> disp_roi("roi", *imgROI, 1, 0, colMap_gray);
                disp_roi.show();
                
                /* Main loop */
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
                    imgComp.execute();
                    
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
            
            
            /* Phase Singurality Analysis */
            void psdetect(void)
            {
                
                int hbt_compRate = this->cam->width / hbt_size + ( this->cam->width % hbt_size == 0 ? 0 : 1);
                char buf[255];
                
                /* Camera */
                RawFileCamera<double> camOpt(
                    this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                    this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );
                RawFileCamera<double> camPhase(
                    this->cam->width/hbt_compRate, this->cam->height/hbt_compRate, std::numeric_limits<double>::digits, this->cam->fps,
                    this->dstDir, fmt_raw_hbt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );
                RawFileCamera<unsigned char> camRoi(
                    this->cam->width/hbt_compRate, this->cam->height/hbt_compRate, std::numeric_limits<char>::digits, this->cam->fps,
                    this->dstDir, fmt_raw_roh, 1,1,1);
                
                /* Analyzer */
                MorphImage<unsigned char>            imgRoiOpen( camRoi, psp_closeRoi);
                MorphImage<unsigned char>            imgRoiClose( imgRoiOpen, -2 * psp_closeRoi);
                MedianFilter<double>                 imgMed( camPhase, psp_closeRoi );
                PhaseMedianFilter<double>            imgFil( imgMed, psp_medianSize );
                DivPhaseSingularityAnalyzer<double>  imgDivPSP( imgFil );
                LabelImage                           imgLabel(dynamic_cast<Image<unsigned char>&>(imgDivPSP));
                
                /* ROI setting */
                camRoi.capture();
                imgRoiOpen.execute();
                imgRoiClose.execute();
                for( int j = 0; j < camRoi.height; j++ ){
                    if( j <= camRoi.height * psp_roiMarginTop || j >= camRoi.height * (1 - psp_roiMarginBottom)) {
                        for( int i = 0; i < camRoi.width; i++ ){
                            imgRoiClose.setValue( i, j, 0);
                        }
                    }
                }
                for( int i = 0; i < camRoi.width; i++ ){
                    if( i <= camRoi.height * psp_roiMarginLeft || i >= camRoi.height * (1 - psp_roiMarginRight)) {
                        for( int j = 0; j < camRoi.height; j++ ){
                            imgRoiClose.setValue( i, j, 0);
                        }
                    }
                }
                imgDivPSP.setRoi ( imgRoiClose );
                
                /* Display */
                Display<double>         disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
                Display<double>         dispFil("Filtered Phase Map", imgFil, M_PI, -M_PI, colMap_hsv);
                Display<double>         dispDiv("Phase div", imgDivPSP.imgDiv, 1.0, 0, colMap_gray);

                /* Log file */
                sprintf( buf, fmt_log_psp.c_str(), this->dstDir.c_str() );
                ofstream ofs(buf);
                if( !ofs ) throw "failed to open file with format " + fmt_log_psp;
                
                /* Main loop */
                camPhase.initialize();
                while( stop != camPhase.state && error != camPhase.state ){
                    
                    camPhase.capture();
                    if( camPhase.f_tmp - camPhase.f_start <= hbt_minPeakDistance * 4 ) continue;
                    camOpt.capture();
                    imgMed.execute();
                    imgFil.execute();
                    imgDivPSP.execute();
                    imgLabel.execute();
                    
                    /* output PS info.*/
                    ofs << camPhase.getTime() << "," ;
                    for( std::vector<MLARR::Basic::Point<double> >::iterator it = imgLabel.vec_ps.begin(); it != imgLabel.vec_ps.end(); it++ ){
                        //dispExc.drawRect( it->getX(), it->getY(), 2, white );
                        dispFil.drawRect( it->getX(), it->getY(), 2, white );
                        ofs << it->getX()*hbt_compRate << "," << it->getY()*hbt_compRate << ",";
                    }
                    ofs << std::endl;
                    
                    /* show images.*/
                    disp_opt.show( camPhase.getTime(), red);
                    dispFil.show( camPhase.getTime(), red);
                    dispDiv.show( camPhase.getTime(), red );
                    
                    /* save images */
                    dispFil.save(this->dstDir, fmt_jpg_hbf, camPhase.getTime());
                    
                    //cvWaitKey(-1);
                }
                
                ofs.close();
                
            };
            
            void isochronal(void)
            {
                
                int hbt_compRate = this->cam->width / hbt_size + ( this->cam->width % hbt_size == 0 ? 0 : 1);
                char buf[255];
                
                /* Camera */
                RawFileCamera<double> camOpt(
                                             this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                                             this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );
                RawFileCamera<double> camPhase(
                                               this->cam->width/hbt_compRate, this->cam->height/hbt_compRate, std::numeric_limits<double>::digits, this->cam->fps,
                                               this->dstDir, fmt_raw_hbt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );
                RawFileCamera<unsigned char> camRoi(
                                                    this->cam->width/hbt_compRate, this->cam->height/hbt_compRate, std::numeric_limits<char>::digits, this->cam->fps,
                                                    this->dstDir, fmt_raw_roh, 1,1,1);
                
                /* Analyzer */
                MorphImage<unsigned char>            imgRoiOpen( camRoi, psp_closeRoi);
                MorphImage<unsigned char>            imgRoiClose( imgRoiOpen, -2 * psp_closeRoi);
                MedianFilter<double>                 imgMed( camPhase, psp_closeRoi );
                PhaseMedianFilter<double>            imgFil( imgMed, psp_medianSize );
                PhaseRangeDetector<double>           imgFront( imgFil, M_PI * psp_phaseFrontMean, psp_phaseFrontRange);
                MorphImage<unsigned char>            imgFrontOpen(imgFront, 5);
                MorphImage<unsigned char>            imgFrontClose(imgFrontOpen, -4);
                PositiveFilter<unsigned char>        filPos(0);
                ActivationTimeMap<
                unsigned char,
                PositiveFilter<unsigned char> >  imgActTime(imgFrontClose, filPos, hbt_minPeakDistance);
                MedianFilter<unsigned short>         imgATMFil( imgActTime, 5 );
                Image<unsigned char>                 imgIso( imgFront.height, imgFront.width, 0 );
                
                /* ROI setting */
                camRoi.capture();
                imgRoiOpen.execute();
                imgRoiClose.execute();
                for( int j = 0; j < camRoi.height; j++ ){
                    if( j <= camRoi.height * psp_roiMarginTop || j >= camRoi.height * (1 - psp_roiMarginBottom)) {
                        for( int i = 0; i < camRoi.width; i++ ){
                            imgRoiClose.setValue( i, j, 0);
                        }
                    }
                }
                for( int i = 0; i < camRoi.width; i++ ){
                    if( i <= camRoi.height * psp_roiMarginLeft || i >= camRoi.height * (1 - psp_roiMarginRight)) {
                        for( int j = 0; j < camRoi.height; j++ ){
                            imgRoiClose.setValue( i, j, 0);
                        }
                    }
                }
                imgFront.setRoi  ( imgRoiClose );
                imgActTime.setRoi( imgRoiClose );
                
                /* Display */
                Display<double>         disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
                Display<double>         dispFil("Filtered Phase Map", imgFil, M_PI, -M_PI, colMap_hsv);
                Display<unsigned short> disp_atm("activation time", imgATMFil, psp_isoMax, 0, colMap_gray);
                Display<unsigned char>  dispIso("Isochronal", imgIso, 1, 0, colMap_gray);
                
                /* Log file */
                sprintf( buf, fmt_log_psp.c_str(), this->dstDir.c_str() );
                ofstream ofs(buf);
                if( !ofs ) throw "failed to open file with format " + fmt_log_psp;
                
                /* Main loop */
                camPhase.initialize();
                while( stop != camPhase.state && error != camPhase.state ){
                    
                    camPhase.capture();
                    if( camPhase.f_tmp - camPhase.f_start <= hbt_minPeakDistance * 4 ) continue;
                    camOpt.capture();
                    imgMed.execute();
                    imgFil.execute();
                    imgFront.execute();
                    imgFrontOpen.execute();
                    imgFrontClose.execute();
                    imgActTime.execute();
                    imgATMFil.execute();
                    imgIso.clear(0);
                    
                    /* show images.*/
                    disp_opt.show( camPhase.getTime(), red);
                    dispFil.show( camPhase.getTime(), red);
                    disp_atm.show();
                    for( int i = psp_isoInterval; i < psp_isoMax; i+=psp_isoInterval){ // draw isochronal lines
                        RangeDetector<unsigned short>        imgWF( imgATMFil, i-1, i+1 );
                        BinaryThinLine<unsigned char>        imgWFLine( imgWF );
                        imgWF.execute();
                        imgWFLine.execute();
                        cv::Vec3b color;
                        IO::brendColor( blue, green, i / static_cast<double>(psp_isoMax), color);
                        dispIso.drawMask(imgWFLine, color);
                    }
                    dispIso.show();
                    
                    /* save images */
                    disp_opt.save(this->dstDir, fmt_jpg_psp, camPhase.getTime());
                    dispFil.save(this->dstDir, fmt_jpg_hbf, camPhase.getTime());
                    dispIso.save(this->dstDir, fmt_jpg_iso, camPhase.getTime());
                    
                    //cvWaitKey(-1);
                }
                
                ofs.close();
                
            };

            
            void SimplePhaseAnalysis(){
                
                /* Camera */
                RawFileCamera<double> imgOpt(
                     this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                     this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip,this->cam->f_stop );

                /* Analyzer */
                ImageShrinker<double> optSh1( imgOpt );
                ImageShrinker<double> optSh2( dynamic_cast<Image<double>&>(optSh1) );
                ImageShrinker<double> optSh3( dynamic_cast<Image<double>&>(optSh2) );
                ImageShrinker<double> optSh4( dynamic_cast<Image<double>&>(optSh3) );
                SimplePhaseSingularityAnalyzer<double> spsEng1( optSh1, MLARR::Analyzer::coefficients.vec_spFIR, 5 );
                SimplePhaseSingularityAnalyzer<double> spsEng2( optSh2, MLARR::Analyzer::coefficients.vec_spFIR, 5 );
                SimplePhaseSingularityAnalyzer<double> spsEng3( optSh3, MLARR::Analyzer::coefficients.vec_spFIR, 5 );
                SimplePhaseSingularityAnalyzer<double> spsEng4( optSh4, MLARR::Analyzer::coefficients.vec_spFIR, 5 );
                
                ImageDoubler<unsigned char> spsDouble4( spsEng4 );
                BinaryAnd<unsigned char> sps43( spsEng3, spsDouble4 );
                ImageDoubler<unsigned char> spsDouble43( sps43 );
                BinaryAnd<unsigned char> sps432( spsEng2, spsDouble43 );
                ImageDoubler<unsigned char> spsDouble432( sps432 );
                BinaryAnd<unsigned char> sps4321( spsEng1, spsDouble432 );
                
                /* Display */
                Display<double> disp_opt("optical", imgOpt, 1.0, 0.0, colMap_orange);
                Display<double> disp_opt_sh1("1", optSh1, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
                Display<double> disp_opt_sh2("2", optSh2, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
                Display<double> disp_opt_sh3("3", optSh3, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
                Display<double> disp_opt_sh4("4", optSh4, 1.0, 0.0, colMap_orange, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps1( "sps1", spsEng1, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps2( "sps2", spsEng2, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps3( "sps3", spsEng3, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps4( "sps4", spsEng4, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps43( "sps43", sps43, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps432( "sps432", sps432, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                Display<unsigned char> disp_sps4321( "sps4321", sps4321, 1, 0, colMap_gray, this->cam->width, this->cam->height );
                
                /* Main loop */
                while( stop != imgOpt.state && error != imgOpt.state ){
                    this->cam->capture();
                    optSh1.execute();
                    optSh2.execute();
                    optSh3.execute();
                    optSh4.execute();
                    spsEng4.execute();
                    spsEng3.execute();
                    spsEng2.execute();
                    spsEng1.execute();
                    spsDouble4.execute();
                    sps43.execute();
                    spsDouble43.execute();
                    sps432.execute();
                    spsDouble432.execute();
                    sps4321.execute();
                    disp_opt.show(imgOpt.getTime(), white);
                    disp_opt_sh1.show(imgOpt.getTime(), white);
                    disp_opt_sh2.show(imgOpt.getTime(), white);
                    disp_opt_sh3.show(imgOpt.getTime(), white);
                    disp_opt_sh4.show(imgOpt.getTime(), white);
                    disp_sps1.show(imgOpt.getTime(), red);
                    disp_sps2.show(imgOpt.getTime(), red);
                    disp_sps3.show(imgOpt.getTime(), red);
                    disp_sps4.show(imgOpt.getTime(), red);
                    disp_sps43.show(imgOpt.getTime(), red);
                    disp_sps432.show(imgOpt.getTime(), red);
                    disp_sps4321.show(imgOpt.getTime(), red);
                    if( 'c' == cv::waitKey(5) ) break;
                }
                
            };
            
            void execute(void){
                
                if( this->vec_menu.size() == 1 && vec_menu[0] == "full" ){
                    vec_menu.push_back("revnorm");
                    vec_menu.push_back("hilbert");
                    vec_menu.push_back("psdetect");
                    vec_menu.push_back("isochronal");
                }
                
                for( std::vector<std::string>::iterator it = vec_menu.begin(); it != vec_menu.end(); it++ ){
                    if(*it == "revnorm")
                        revNorm();
                    if(*it == "hilbert")
                        hilbertPhase();
                    if(*it == "psdetect")
                        psdetect();
                    if(*it == "isochronal")
                        isochronal();
                }
                
            };
            
        };
    }
}


#endif
