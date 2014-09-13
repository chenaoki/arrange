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
#include "MovieAnalyzer.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "picojson.h"
#include "Optical.h"
#include "Engine.h"
#include "AVTCamera.h"

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
            std::string camType;
            ICamera<T>* cam = NULL;
        public:
            
            FeedbackEngine(std::string& paramFilePath) : Engine(paramFilePath){
                
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath) ;
                
                /* create camera object */
                camType = obj["camType"].get<std::string>();
                cam = static_cast<ICamera<T>*>( MLARR::IO::CameraFactory::create(this->camType, paramFilePath) );
                if( NULL == cam ){
                    throw string("Failed to locate avt camera.");
                }
                
            };
            
            ~FeedbackEngine(){delete cam;};
            
            void execute(void){
                
				MaxImageAnalyzer<T> maxEng(0, *(this->cam));
				MinImageAnalyzer<T> minEng(( 1 << this->cam->bits )-1, *(this->cam));
                OpticalImageAnalyzer<T> imgOpt(*(this->cam), maxEng, minEng, 20);
                ImageThinOut<double> imgHalf(imgOpt);
                ImageThinOut<double> imgQuarter(imgHalf);
                
                SimplePhaseSingularityAnalyzer<double> imgSP( imgQuarter, MLARR::Analyzer::coefficients.vec_spFIR );
                PyramidDetector< double, ImageThinOut<double>, SimplePhaseSingularityAnalyzer<double> > imgPyr( imgHalf, &imgSP, 3);
                
                Display<T> disp_cam( "camera input", *cam, ( 1 << this->cam->bits )-1, 0, colMap_gray, cam->width, cam->height );
                Dumper<T> dump_cam( *(this->cam), this->dstDir, "%s/%05d.raw");
                

#if 0

                Display<double> disp_opt("optical", imgOpt, 1.0, 0.0, colMap_orange);
                vector< IO::Display<double>* > vec_disp_double;
                vector< IO::Display<unsigned char>* > vec_disp_uchar;
                for( vector<ImageThinOut<double>*>::iterator it = imgPyr.vec_shrinker.begin(); it != imgPyr.vec_shrinker.end(); it++ ){
                    ostringstream name;
                    name << "pyr down " << std::distance(imgPyr.vec_shrinker.begin(), it);
                    vec_disp_double.push_back( new Display<double>( name.str().c_str(), *(*it), 1.0, 0.0, colMap_orange));
                }
                for( vector<SimplePhaseSingularityAnalyzer<double>*>::iterator it = imgPyr.vec_analyzer.begin(); it != imgPyr.vec_analyzer.end(); it++ ){
                    ostringstream name;
                    name << "pyr output " << std::distance(imgPyr.vec_analyzer.begin(), it);
                    vec_disp_uchar.push_back( new Display<unsigned char>( name.str().c_str(), (*it)->imgSP, 4, 0, colMap_orange));
                }
                
#endif
                
                if( cam ){
                    
                    //int cnt = 0;
                    //struct timeval s, t;
                    while ( stop != cam->state ) {

                        cam->capture();

                        disp_cam.show(cam->getTime(), white);

#if 1
                        dump_cam.dump(this->cam->f_tmp);
                        // disp_cam.save( this->dstDir, string("%s/avt_%d.jpg"), cam->f_tmp );
#endif

                        
#if 0
                        imgHalf.execute();
                        imgQuarter.execute();
                        
                        if( cam->f_tmp == 200 )
                            imgOpt.updateRange();
                        
                        if( cam->f_tmp < 200 ){
                            maxEng.execute();
                            minEng.execute();
                            disp_cam.show( cam->getTime(), white);
                        }else{
                            //if( cnt == 0 ) gettimeofday(&s, NULL);
                            cnt++;
                            imgOpt.execute();
                            imgSP.execute();
                            imgPyr.execute();
                            
                            disp_opt.show( cam->getTime(), white);

                            for( int i = 0; i < vec_disp_double.size(); i++){
                                vec_disp_double[i]->show();
                            }
                            for( int i = 0; i < vec_disp_uchar.size(); i++){
                                vec_disp_uchar[i]->show();
                            }
                            cout << cam->f_tmp << endl;
                            cvWaitKey(-1);
                        }
#endif
                        

                        
                    }
                    /*
                    gettimeofday(&t, NULL);
                    double msecs = ( t.tv_sec - s.tv_sec )  * 1000 + static_cast<double>(t.tv_usec - s.tv_usec) / 1000;
                    cout << msecs<< "msec for " << cnt << " times loop (" << static_cast<double>( msecs ) / cnt << ")" << endl;
                    */
                }
                
                
            };
            
        };
        
    }
}

#endif
