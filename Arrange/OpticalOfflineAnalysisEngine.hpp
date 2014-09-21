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
#include "MovieAnalyzer.h"
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
		const std::string fmt_jpg_ecg("%s/jpg/ecg/ecg_%05d.jpg");
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
            int psp_openRoi;
            int psp_openPS;
            int psp_closeRoi;
            int psp_medianSize;
            double psp_roiMarginTop;
            double psp_roiMarginBottom;
            double psp_roiMarginLeft;
            double psp_roiMarginRight;
            std::string camType;
            
        public:
            OpticalOfflineAnalysisEngine(std::string& paramFilePath) : Engine(paramFilePath), cam(NULL), _electrodes(), vec_menu()
            {
                
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                
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
                temp = this->dstDir + "/jpg/ecg";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/log";
                mkdir( temp.c_str() , 0777);
                

                
                /* create camera object */
                camType = obj["camType"].get<std::string>();
                if( NULL == ( this->cam = static_cast<RawFileCamera<T>*>( MLARR::IO::CameraFactory::create(this->camType, paramFilePath)))){
                    throw "failed to create camera object";
                }
                
                /* load parameter file (JSON) */
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
                if( !ofs ) throw string("failed to open file with format ") + fmt_log_phs;
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
            
            void monitorElecExcitation(void)
            {
                /* Camera */
                RawFileCamera<double> camOpt(
                 this->cam->width, this->cam->height, std::numeric_limits<double>::digits, this->cam->fps,
                 this->dstDir, fmt_raw_opt, this->cam->f_start,this->cam->f_skip, this->cam->f_stop );
                
                /* Analyzer */
                vector<double> coef;
                const int coefSize = 10;
                for( int i = 0; i < coefSize; i++){
                    if( i == 0 ){
                        coef.push_back( -1.0);
                    }else if( i + 1 == coefSize ){
                        coef.push_back( 1.0 );
                    }else{
                        coef.push_back( 0.0 );
                    }
                }
                vector<ActivationMonitorElectrode> vecAME;
                for( int i = 0; i< this->_electrodes.size(); i++){
                    // vecAME.push_back( ActivationMonitorElectrode( _electrodes[i], coefficients.vec_spFIR));
                    vecAME.push_back( ActivationMonitorElectrode( _electrodes[i], coef, 0.3, 30));
                }
                
                /* Display */
                Display<double> disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
                
                /* Log file */
                char buf[255];
                sprintf( buf, fmt_log_act.c_str(), this->dstDir.c_str() );
                ofstream ofs(buf);
                if( !ofs ) throw string("failed to open file with format ") + fmt_log_phs;
                
                /* Main loop */
                while( stop != camOpt.state && error != camOpt.state ){
                    camOpt.capture();
                    disp_opt.show( camOpt.getTime(), white );
                    
                    for( vector<ActivationMonitorElectrode>::iterator it = vecAME.begin(); it != vecAME.end(); it++ ){
                        double val = *camOpt.getRef(it->getX(), it->getY());
                        it->monitor(camOpt.getTime(), val);
                        ofs << it->getID() << ","  << val << "," << it->getFilteredValue() << "," << it->getPastTime() << ",";
                    }
                    ofs << endl;
                }
                
                ofs.close();
                return;
            }
            
            /* Phase Singurality Analysis */
            void phaseSingularityAnalysis(void)
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
                MorphImage<unsigned char>            preOpenRoi(camRoi, psp_openRoi);
                MorphImage<unsigned char>            closeRoi(preOpenRoi, -1*psp_closeRoi);
                MedianFilter<double>                 imgMed( dynamic_cast<Image<double>&>(camPhase), psp_medianSize);
                PhaseSpacialFilter<double>           imgFil( dynamic_cast<Image<double>&>(imgMed), 5, 5, coefficients.vec_gaussian_5x5);
                //AdjPhaseSingularityAnalyzer<double>  imgPS(dynamic_cast<Image<double>&>(imgFil));
                //PyramidDetector< double, ImageThinOut<double>, AdjPhaseSingularityAnalyzer<double> > imgPyrPS( imgFil, &imgPS, 2 );
                //MorphImage<unsigned char>            imgOpenPS( imgPS, psp_openPS );
                PhaseRangeDetector<double>           imgFront(imgFil, - M_PI / 2, M_PI / 16);
                PhaseRangeDetector<double>           imgTail(imgFil, M_PI / 2, M_PI / 24);
                PhaseRangeDetector<double>           imgExcitable(imgFil, 0, 9 * M_PI / 20);
                MorphImage<unsigned char>            imgFrontOpen(imgFront, 5);
                MorphImage<unsigned char>            imgTailOpen (imgTail, 5);
                MorphImage<unsigned char>            imgFrontClose(imgFrontOpen, -4);
                MorphImage<unsigned char>            imgTailClose(imgTailOpen, -4);
                BinaryAnd<unsigned char>             imgAnd( imgFrontClose, imgTailClose, 0 );
                BinaryAdjacent<unsigned char>        imgAdj( imgFrontClose, imgTailClose, 5 );
                MorphImage<unsigned char>            imgAdjClose(imgAdj, -1);
                LabelImage                           imgLabel(dynamic_cast<Image<unsigned char>&>(imgAdjClose));
                
                /* ROI setting */
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
                //imgPyrPS.setRoi( dynamic_cast<const Image<unsigned char>&>(closeRoi) );
                imgAdj.setRoi( camRoi );
                imgFront.setRoi( camRoi );
                imgTail.setRoi( camRoi );
                
                /* Display */
                Display<double> disp_opt("optical", camOpt, 1.0, 0.0, colMap_orange);
                Display<double> dispFil  ("Filtered Phase Map", imgFil, M_PI, -M_PI, colMap_hsv);
                //Display<unsigned char> dispPS   ("Phase Singurality Image", imgOpenPS, 1, 0, colMap_gray);
                std::vector< IO::Display<unsigned char>* > vec_disp;
                std::vector< IO::Display<double>* > vec_disp_shrink;
                /*for( int i = 0; i < imgPyrPS.vec_analyzer.size(); i++){
                    sprintf(buf, "Pyramid analyze %d", i);
                    AdjPhaseSingularityAnalyzer<double>* ptr = imgPyrPS.vec_analyzer[i];
                    vec_disp.push_back(new IO::Display<unsigned char>( std::string(buf), *ptr, 1, 0, IO::colMap_gray, ptr->width, ptr->height ));
                }
                for( int i = 0; i < imgPyrPS.vec_shrinker.size(); i++){
                    char buf[128];
                    sprintf(buf, "Pyramid shrink %d", i);
                    ImageThinOut<double>* p = imgPyrPS.vec_shrinker[i];
                    vec_disp_shrink.push_back(new IO::Display<double>( std::string(buf), *p, M_PI, -M_PI, colMap_hsv));
                }*/
                
                //Display<unsigned char> dispFront("wave front", imgFront, 1, 0, colMap_gray);
                //Display<unsigned char> dispTail("wave tail", imgTail, 1, 0, colMap_gray);
                //Display<unsigned char> dispFrontClose("open front", imgFrontClose, 1, 0, colMap_gray);
                //Display<unsigned char> dispTailClose("open tail", imgTailClose, 1, 0, colMap_gray);
                //Display<unsigned char> dispAnd("front & tail", imgAnd, 1, 0, colMap_gray);
                Display<unsigned char> dispExcitable("excitable gap", imgExcitable, 1, 0, colMap_gray);
                //Display<unsigned char> dispAdj("adjacent", imgAdjClose, 1, 0, colMap_gray);
                
                sprintf( buf, fmt_log_psp.c_str(), this->dstDir.c_str() );
                ofstream ofs(buf);
                if( !ofs ) throw "failed to open file with format " + fmt_log_psp;
                
                
                /* Main loop */
                camPhase.initialize();
                while( stop != camPhase.state && error != camPhase.state ){
                    
                    camPhase.capture();
                    camOpt.capture();
                    imgFil.execute();
                    imgMed.execute();
                    //imgPyrPS.execute();
                    //imgOpenPS.execute();
                    imgFront.execute();
                    imgTail.execute();
                    imgFrontOpen.execute();
                    imgTailOpen.execute();
                    imgFrontClose.execute();
                    imgTailClose.execute();
                    imgExcitable.execute();
                    imgAnd.execute();
                    imgAdj.execute();
                    imgAdjClose.execute();
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
                    //dispPS.show( camPhase.getTime(), red);
                    //dispFront.show( camPhase.getTime(), red );
                    //dispTail.show( camPhase.getTime(), red );
                    //dispFrontClose.show( camPhase.getTime(), red );
                    //dispTailClose.show( camPhase.getTime(), red );
                    dispExcitable.drawMask(imgFrontClose, red);
                    dispExcitable.drawMask(imgTailClose, blue);
                    dispExcitable.drawMask(imgAdj, green);
                    dispExcitable.show( camPhase.getTime(), red );
                    //dispAnd.show( camPhase.getTime(), red );
                    //dispAdj.show( camPhase.getTime(), red );
                    
                    /*
                    for( int i = 0; i < vec_disp.size(); i++){
                        vec_disp[i]->show();
                    }
                    for( int i = 0; i < vec_disp_shrink.size(); i++){
                        vec_disp_shrink[i]->show();
                    }*/
                    
                    
                    disp_opt.save(this->dstDir, fmt_jpg_psp, camPhase.getTime());
                    dispFil.save(this->dstDir, fmt_jpg_hbf, camPhase.getTime());
                    dispExcitable.save(this->dstDir, fmt_jpg_ecg, camPhase.getTime());
                    
                    //cvWaitKey(-1);
                }
                
                
                for( int i = 0; i < vec_disp.size(); i++){
                    delete vec_disp[i];
                }
                for( int i = 0; i < vec_disp_shrink.size(); i++){
                    delete vec_disp_shrink[i];
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
                SimplePhaseSingularityAnalyzer<double> spsEng1( optSh1, MLARR::Analyzer::coefficients.vec_spFIR );
                SimplePhaseSingularityAnalyzer<double> spsEng2( optSh2, MLARR::Analyzer::coefficients.vec_spFIR );
                SimplePhaseSingularityAnalyzer<double> spsEng3( optSh3, MLARR::Analyzer::coefficients.vec_spFIR );
                SimplePhaseSingularityAnalyzer<double> spsEng4( optSh4, MLARR::Analyzer::coefficients.vec_spFIR );
                
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
                    //vec_menu.push_back("monitor");
                    vec_menu.push_back("psdetect");
                }
                
                for( std::vector<std::string>::iterator it = vec_menu.begin(); it != vec_menu.end(); it++ ){
                    if(*it == "revnorm")
                        revNorm();
                    if(*it == "hilbert")
                        hilbertPhase();
                    if(*it == "monitor")
                        monitorElecExcitation();
                    if(*it == "psdetect")
                        phaseSingularityAnalysis();
                }
                
            };
            
        };
    }
}


#endif
