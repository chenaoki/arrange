//
//  RealtimeEngine.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/10.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_RealtimeEngine_hpp
#define Arrange_RealtimeEngine_hpp

#include "stdafx.h"

#include "Basic.h"
#include "Analyzer.h"
#include "Hilbert.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "picojson.h"
#include "Optical.h"
#include "Engine.h"
#include "AVTCamera.h"

/* for CvKalman */
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/legacy/legacy.hpp>

using namespace std;
using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Analyzer;
using namespace MLARR::Engine;

namespace MLARR{
	namespace Engine{
        
        template<typename T>
		class FeedbackEngine : public Engine{
            
        protected:
            string fmt_raw_cam;
            string fmt_jpg_cam;
            string fmt_jpg_det;
            string fmt_jpg_psp;
            string fmt_log_sps;
            
            
        protected:
            enum EExecMode{ e_mode_run=0, e_mode_dump, e_mode_monitor, e_mode_capture };
            std::string camType;
            int nMode;
            int nFrameRange;
            int nPyrDown;
            int psp_procImageSize;
            int psp_winSize;
            int psp_closeRoi;
            T rnm_minDelta;
            double psp_kalmanProcNoiseCov;
            double psp_kalmanMeasNoiseCov;
            double psp_roiMarginTop;
            double psp_roiMarginBottom;
            double psp_roiMarginLeft;
            double psp_roiMarginRight;

            ICamera<T>* cam = NULL;
            
        public:
            
            FeedbackEngine(std::string& paramFilePath) : Engine(paramFilePath),
            fmt_raw_cam("%s/raw/cam/%06d.raw"),
            fmt_jpg_cam("%s/jpg/cam/%06d.jpg"),
            fmt_jpg_det("%s/jpg/det/%06d.jpg"),
            fmt_jpg_psp("%s/jpg/psp/%06d.jpg"),
            fmt_log_sps("%s/log/sps.log")
            {
                
                /* load parameter file (JSON) */
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                camType = obj["camType"].get<std::string>();
                picojson::object fb = obj["feedback"].get<picojson::object>();
                nMode = atoi( fb["mode"].to_str().c_str() );
                nFrameRange = atoi( fb["frameRange"].to_str().c_str() );
                nPyrDown = atoi( fb["pyramidDown"].to_str().c_str() );
                rnm_minDelta = atoi( fb["minDelta"].to_str().c_str() );
                psp_procImageSize = atoi( fb["procImageSize"].to_str().c_str() );
                psp_winSize = atoi( fb["winSize"].to_str().c_str() );
                psp_closeRoi = atoi( fb["closeRoi"].to_str().c_str() );
                picojson::object psp_kalman = fb["kalman"].get<picojson::object>();
                psp_kalmanProcNoiseCov = atof( psp_kalman["procNoiseCov"].to_str().c_str() );
                psp_kalmanMeasNoiseCov = atof( psp_kalman["measNoiseCov"].to_str().c_str() );
                picojson::object psp_roi = fb["roiMargin"].get<picojson::object>();
                psp_roiMarginTop = atof( psp_roi["top"].to_str().c_str() );
                psp_roiMarginBottom = atof( psp_roi["bottom"].to_str().c_str() );
                psp_roiMarginLeft = atof( psp_roi["left"].to_str().c_str() );
                psp_roiMarginRight = atof( psp_roi["right"].to_str().c_str() );
                
                /* make output directories */
                std::string temp;
                temp = this->dstDir + "/raw";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/raw/cam";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/cam";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/det";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/psp";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/jpg/pyr";
                mkdir( temp.c_str() , 0777);
                temp = this->dstDir + "/log";
                mkdir( temp.c_str() , 0777);
                char buf[256];
                for( int p = 0; p < nPyrDown; p++){
                    sprintf( buf, "%s/jpg/pyr/%02d", this->dstDir.c_str(), p);
                    mkdir( buf, 0777);
                }
                
                /* create camera object */
                cam = static_cast<ICamera<T>*>( MLARR::IO::CameraFactory::create(this->camType, paramFilePath) );
                if( NULL == cam ){
                    throw string("Failed to locate avt camera.");
                }
                
                
            };
            
            ~FeedbackEngine(){delete cam;};
            
            void execute(void){

                char buf[256];
                
                /* ImageAnalyzer */
                std::vector< Image<T>* > vecCamProc;
                vecCamProc.push_back(this->cam);
                while( vecCamProc.back()->width > psp_procImageSize){
                    vecCamProc.push_back( dynamic_cast< Image<T>* >(new ImageShrinker<T>( *vecCamProc.back() )));
                }
                Image<T>* imgCam = vecCamProc.back();
                
                MaxImageAnalyzer<T>                    maxEng( 0, *imgCam );
                MinImageAnalyzer<T>                    minEng( std::numeric_limits<T>::max(), *imgCam );
                OpticalImageAnalyzer<T>                imgOpt( *imgCam, maxEng, minEng, rnm_minDelta);
                MorphImage<unsigned char>              imgRoiOpen( imgOpt.getRoi(), psp_closeRoi);
                MorphImage<unsigned char>              imgRoiClose( imgRoiOpen, -2 * psp_closeRoi);
                SpacialFilter<T>                       imgFil( *imgCam, 5, 5, coefficients.vec_gaussian_5x5);
                SimplePhaseSingularityAnalyzer<T>      imgSPS( imgFil, MLARR::Analyzer::coefficients.vec_spFIR, psp_winSize );
                PyramidDetector<
                    ImageThinOut<T>,
                    SimplePhaseSingularityAnalyzer<T>,
                    T,  unsigned char>                 imgPyr( imgFil, &imgSPS, nPyrDown);
                ImageCOG<unsigned char>                imgCOG( imgPyr );
                
                /* Kalman Filter setting */
                CvKalman *kalman = cvCreateKalman(4, 2);
                cvSetIdentity(kalman->measurement_matrix, cvRealScalar(1.0));
                cvSetIdentity(kalman->process_noise_cov, cvRealScalar(psp_kalmanProcNoiseCov));
                cvSetIdentity(kalman->measurement_noise_cov, cvRealScalar(psp_kalmanMeasNoiseCov));
                cvSetIdentity(kalman->error_cov_post, cvRealScalar(1.0));
                kalman->DynamMatr[0]  = 1.0; kalman->DynamMatr[1]  = 0.0; kalman->DynamMatr[2]  = 1.0; kalman->DynamMatr[3]  = 0.0;
                kalman->DynamMatr[4]  = 0.0; kalman->DynamMatr[5]  = 1.0; kalman->DynamMatr[6]  = 0.0; kalman->DynamMatr[7]  = 1.0;
                kalman->DynamMatr[8]  = 0.0; kalman->DynamMatr[9]  = 0.0; kalman->DynamMatr[10] = 1.0; kalman->DynamMatr[11] = 0.0;
                kalman->DynamMatr[12] = 0.0; kalman->DynamMatr[13] = 0.0; kalman->DynamMatr[14] = 0.0; kalman->DynamMatr[15] = 1.0;
                
                /* Dumper */
                Dumper<T> dump_cam( *(this->cam), this->dstDir, this->fmt_raw_cam);
                Dumper<unsigned char> dump_roi( imgOpt.getRoi(), this->dstDir, fmt_raw_roi);

                /* Log file */
                sprintf( buf, this->fmt_log_sps.c_str(), this->dstDir.c_str() );
                ofstream ofs(buf);
                if( !ofs ) throw "failed to open file with format " + fmt_log_psp;

                
                /* Display */
                Display<T> disp_cam( "camera input", *imgCam, ( 1 << this->cam->bits )-1, 0, colMap_gray, imgCam->width, imgCam->height );
                Display<double> disp_opt( "optical", imgOpt, 1.0, 0.0, colMap_orange);
                Display<unsigned char> disp_pyr("Pyramid output", imgPyr, nPyrDown, 0, colMap_gray);
                Display<unsigned char> disp_roi("Closed ROI", imgOpt.getRoi(), 1, 0, colMap_gray);
                vector< IO::Display<unsigned char>* > vec_disp_sp;
                typedef SimplePhaseSingularityAnalyzer<T> TANALYZER;
                typedef typename std::vector< TANALYZER* >::iterator TANALYZER_IT;
                std::vector< TANALYZER* >& vec = imgPyr.vec_analyzer;
                for( TANALYZER_IT it = vec.begin(); it != vec.end(); it++ ){
                    ostringstream name;
                    name << "pyr simple phase " << std::distance(imgPyr.vec_analyzer.begin(), it);
                    vec_disp_sp.push_back( new Display<unsigned char>( name.str().c_str(), (*it)->imgSP, 4, 0, colMap_hsv));
                }
                
                /* Window size setting */
                for( int i = 1; i < imgPyr.vec_analyzer.size(); i++){
                    imgPyr.vec_analyzer[i]->setWinSize( imgPyr.vec_analyzer[i-1]->winSize / 2 );
                }
                
                /* Main loop */
                double msecs = 0.0;
                if( cam ){
                    
                    int cnt = 0;
                    struct timeval s, t;
                    bool flgRange = false;
                    while ( stop != cam->state ) {

                        /* Capture */
                        cam->capture();
                        for(int i = 1; i < vecCamProc.size(); i++){
                            dynamic_cast< ImageAnalyzer<T, T>* >( vecCamProc[i] )->execute();
                        }
                        
                        if( e_mode_run != nMode && e_mode_capture != nMode ){
                            disp_cam.show(cam->getTime(), white);
                        }
                        if( e_mode_monitor == nMode ) continue;
                        if( e_mode_dump == nMode ){
                            dump_cam.dump();
                        }
                        if( e_mode_capture == nMode ) continue;

                        if( !flgRange ){
                        
                            /* Range Detection */
                            if( cam->f_tmp < this->nFrameRange ){
                                maxEng.execute();
                                minEng.execute();
                                continue;
                            }
                            imgOpt.updateRange();
                            flgRange = true;
                            disp_roi.show();
                            disp_roi.save(this->dstDir, "%s/roi.jpg" );
                            
                            /* ROI setting for pyramid */
                            imgRoiOpen.execute();
                            imgRoiClose.execute();
                            for( int j = 0; j < imgRoiClose.height; j++ ){
                                if( j <= imgRoiClose.height * psp_roiMarginTop || j >= imgRoiClose.height * (1 - psp_roiMarginBottom)) {
                                    for( int i = 0; i < imgRoiClose.width; i++ ){
                                        imgRoiClose.setValue( i, j, 0);
                                    }
                                }
                            }
                            for( int i = 0; i < imgRoiClose.width; i++ ){
                                if( i <= imgRoiClose.height * psp_roiMarginLeft || i >= imgRoiClose.height * (1 - psp_roiMarginRight)) {
                                    for( int j = 0; j < imgRoiClose.height; j++ ){
                                        imgRoiClose.setValue( i, j, 0);
                                    }
                                }
                            }
                            imgPyr.setRoi(dynamic_cast<Image<unsigned char>&>(imgRoiClose));
                            imgCOG.setRoi(imgRoiClose);
                            
                            
                        }
                        
                        /* Time count start */
                        gettimeofday(&s, NULL);
                        cnt++;
                        
                        /* Pre-operation */
                        imgOpt.execute();
                        
                        /* PSP Detection */
                        imgFil.execute();
                        imgPyr.execute();
                        imgPyr.mergeSum(); // Merge pyramid detection result
                        imgCOG.execute();
                        
                        /* Kalman Filtering */
                        if( imgCOG.x >= 0 && imgCOG.y >= 0 ){
                            float meas[] = {static_cast<float>(imgCOG.x), static_cast<float>(imgCOG.y)};
                            CvMat measMat = cvMat(2, 1, CV_32FC1, meas);
                            cvKalmanCorrect(kalman, &measMat); // correction phase
                        }
                        const CvMat *predMat = cvKalmanPredict(kalman); // prediction phase
                        
                        /* Log PS info.*/
                        ofs << cam->getTime();
                        ofs << "," << predMat->data.fl[0];
                        ofs << "," << predMat->data.fl[1];
                        ofs << std::endl;
                        
                        /* Show and save images */
                        if( e_mode_dump == nMode ){
                            disp_opt.drawRect( (int)predMat->data.fl[0], (int)predMat->data.fl[1], 2, MLARR::IO::green );
                            disp_opt.show( cam->getTime(), white);
                            disp_opt.save( this->dstDir, this->fmt_jpg_det );
                            for( int i = 0; i < vec_disp_sp.size(); i++){
                                sprintf( buf, "%%s/jpg/pyr/%02d/%%06d.jpg", i);
                                vec_disp_sp[i]->show(cam->getTime(), white);
                                vec_disp_sp[i]->save(this->dstDir, buf );
                            }
                            disp_pyr.show(cam->getTime(), white);
                            disp_pyr.save(this->dstDir, this->fmt_jpg_psp);
                            cvWaitKey(5);
                        }
                        gettimeofday(&t, NULL);
                        msecs += ( t.tv_sec - s.tv_sec )  * 1000 + static_cast<double>(t.tv_usec - s.tv_usec) / 1000;
                    }
                    
                    cout << msecs<< " / " << cnt << " = " << static_cast<double>( msecs ) / cnt << endl;
                    
                    
                }
                
                /* Terminate */
                ofs.close();
                
                
            };
            
        };
        
    }
}

#endif
