//
//  Optical.h
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/04.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_Optical_h
#define Arrange_Optical_h

#include "stdafx.h"
#include "Basic.h"
#include "coefs.h"
#include <float.h>
#include <algorithm>

#include "IO.h"
#include "Analyzer.h"

namespace MLARR{
    
	namespace Analyzer{
        
        /* convert phase value to be in range from -PI to PI */
        inline double phaseComplement(double x){
            int pos = static_cast<int>( x / M_PI);
            return x - pos * 2 * M_PI;
        };
        
		template<class T>
		class OpticalImageAnalyzer : public ImageAnalyzer<T, double>
		{
		private:
			MLARR::Basic::Image<T>& maxImage;
			MLARR::Basic::Image<T>& minImage;
			MLARR::Basic::Image<T>  im_range;
			const T roiThre;
		public:
			explicit OpticalImageAnalyzer(
                                          MLARR::Basic::Image<T>& _srcImg,
                                          MLARR::Basic::Image<T>& _maxImage,
                                          MLARR::Basic::Image<T>& _minImage, const T _roiThre )
            : ImageAnalyzer<T, double>( _srcImg.height, _srcImg.width, 0.0, _srcImg ), maxImage(_maxImage), minImage(_minImage), im_range(_srcImg.height, _srcImg.width, static_cast<T>(0)), roiThre(_roiThre){
                this->updateRange();
			};
			virtual ~OpticalImageAnalyzer(){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.getRef(w, h)) ){
							T max = *(this->maxImage.getRef(w, h));
							T range = *(this->im_range.getRef(w, h));
							T val = *(this->srcImg.getRef(w, h));
							if( range ){
								this->setValue( w, h, static_cast<double>( max - val ) / range );
							}
						}else{
							this->setValue( w, h, 0.0 );
						}
					}
				}
			};
			void updateRange(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						T range = *(maxImage.getRef(w, h)) - *(minImage.getRef(w, h));
						this->im_range.setValue( w, h, range);
						this->im_roi.setValue( w, h, range > roiThre ? 1 : 0 );
					}
				}
			};
		};
        
        template<class T>
        class PhaseSpacialFilter : public SpacialFilter<T>{
        public:
            explicit PhaseSpacialFilter( MLARR::Basic::Image<T>& _srcImg, const MLARR::Basic::Image<double>& _coeffImg)
            : SpacialFilter<T>( _srcImg, _coeffImg){};
            explicit PhaseSpacialFilter( MLARR::Basic::Image<T>& _srcImg, int filWidth, int filHeight, const std::vector<double>& _coeff)
            : SpacialFilter<T>( _srcImg, filWidth, filHeight, _coeff){};
            ~PhaseSpacialFilter(void){};
        public:
            void execute(void){
				*(dynamic_cast<MLARR::Basic::Image<T>*>(this)) = this->srcImg;
				for( int h = 0; h <= this->height - this->coeffImg.height; h++){
					for( int w = 0; w <= this->width - this->coeffImg.width; w++){
                        int h_c = h + ( this->coeffImg.height - 1 )/2;
                        int w_c = w + ( this->coeffImg.width - 1 )/2;
						if( *(this->im_roi.getRef(w_c, h_c)) ){
                            
                            // search Max, Min
                            double min = 2 * M_PI;
                            for( int i = 0; i < this->coeffImg.width; i++){
								for( int j = 0; j < this->coeffImg.height; j++){
                                    double value = *(this->srcImg.getRef(w+i, h+j));
                                    min = min > value ? value : min;
								}
							}
                            
							// position of center pix
							double value = 0.0;
							for( int i = 0; i < this->coeffImg.width; i++){
								for( int j = 0; j < this->coeffImg.height; j++){
                                    double diff = (*(this->srcImg.getRef(w+i, h+j))) - min;
									value += phaseComplement( diff ) * (*(this->coeffImg.getRef(i, j)));
								}
							}
							this->setValue(w_c, h_c, static_cast<T>(phaseComplement(value+min)));
						}
					}
				}
                
            };
            
        };

        
        template<typename T>
		class SimplePhaseAnalyzer : public ImageAnalyzer<T, unsigned char>{
		public:
			std::vector<double> coef;
			std::vector<MLARR::Basic::Image<T> > buffer;
			MLARR::Basic::Image<char> img_sign;
			enum EPhase{unknown = 0, downstroke, peak, upstroke, bottom};
		public:
            
			explicit SimplePhaseAnalyzer(MLARR::Basic::Image<T>& _srcImg, std::vector<double> _coef)
            : ImageAnalyzer<T, unsigned char>(_srcImg.height, _srcImg.width, unknown, _srcImg), coef(_coef), buffer(), img_sign(_srcImg.height, _srcImg.width, 0){
			};
			
            ~SimplePhaseAnalyzer(void){
				while( buffer.size() > 0){
                    this->buffer.erase(this->buffer.begin());
				}
			};
			
            void execute(void){
				
				typename std::vector<MLARR::Basic::Image<T> >::iterator it_buf;
				typename std::vector<double>::iterator it_coef;
                
				this->buffer.push_back(this->srcImg);
				while( buffer.size() > coef.size()){
					it_buf = this->buffer.begin();
					this->buffer.erase(it_buf);
				}
				if( coef.size() == buffer.size() ){
					for( int h = 0; h < this->height; h++){
						for( int w = 0; w < this->width; w++){
							if( *(this->im_roi.getRef(w, h)) ){
								double val = 0.0;
								it_buf = buffer.begin();
								it_coef = coef.begin();
								for(; it_coef!=coef.end(); it_coef++){
									T pixVal = *((*it_buf).getRef(w, h));
									val += (*it_coef) * pixVal;
									it_buf++;
								}
								char tempSign = val > 0.0 ? 1 : 0 ;
								char lastSign = *(this->img_sign.getRef(w, h));
								this->setValue(w, h, tempSign > 0 ? ( lastSign > 0 ? upstroke : bottom ) : ( lastSign > 0 ? peak : downstroke ));
								this->img_sign.setValue( w, h, tempSign);
							}else{
								this->setValue(w, h, 0);
							}
						}
					}
				}
			}
            
		};
        
		template <class T>
		class SimplePhaseSingularityAnalyzer : public ImageAnalyzer<T, unsigned char>{
        public:
            SimplePhaseAnalyzer<T> imgSP;
		
        public:

			explicit SimplePhaseSingularityAnalyzer( MLARR::Basic::Image<T>& _srcImg, std::vector<double> _coef)
            : ImageAnalyzer<T, unsigned char>( _srcImg.height, _srcImg.width, 0, dynamic_cast<MLARR::Basic::Image<T>&>(_srcImg)), imgSP(_srcImg, _coef)
            {};
            
            explicit SimplePhaseSingularityAnalyzer(MLARR::Basic::Image<T>& _srcImg, SimplePhaseSingularityAnalyzer<T>& pre)
            : ImageAnalyzer<T, unsigned char>( _srcImg.height, _srcImg.width, 0, dynamic_cast<MLARR::Basic::Image<T>&>(_srcImg)), imgSP(_srcImg, pre.imgSP.coef)
            {};
            
			~SimplePhaseSingularityAnalyzer(void){};
			
            void execute(void){
				this->clear(0);
                imgSP.execute();
				for( int h = 1; h < this->height - 1; h++){
					for( int w = 1; w < this->width - 1; w++){
						if( *(this->im_roi.getRef(w, h)) ){
							int flag_on[5] = {1,1,1,1,1};
							int flag[5] = {1,0,0,0,0};
							for(int i = -1; i <=1 ; i++){
								for(int j = -1; j <=1; j++){
									char psVal = *this->imgSP.getRef(w+i, h+j);
									if( psVal >= 0 && psVal <= 4){
										flag[psVal] = 1;
									}
								}
							}
							if( 0 == memcmp( flag_on , flag , sizeof(int) * 5 )){
								this->setValue( w, h, 1 );
							}
						}
					}
				}
			};
		};
        
        template <class T>
		class NormalPhaseSingularityAnalyzer : public ImageAnalyzer<T,T>{
        public:
            ImageDiffX<T> imgDiffX;
            ImageDiffY<T> imgDiffY;
            SpacialFilter<T> imgCurlX;
            SpacialFilter<T> imgCurlY;
		public:
			explicit NormalPhaseSingularityAnalyzer( MLARR::Basic::Image<double>& _src )
			:ImageAnalyzer<double,double>( _src.height, _src.width, 0, _src),
            imgDiffX( this->srcImg ), imgDiffY( this->srcImg ),
            imgCurlX( this->imgDiffX, 3, 3, MLARR::Analyzer::coefficients.vec_curlX ),
            imgCurlY( this->imgDiffY, 3, 3, MLARR::Analyzer::coefficients.vec_curlY ) {
            };
			~NormalPhaseSingularityAnalyzer(void){};
		public:
			void execute(void){
                
                this->imgDiffX.execute();
                this->imgDiffY.execute();
				for( int h = 0; h <this->height; h++){
					for( int w = 0; w <this->width; w++){
						if( *(this->im_roi.getRef(w, h)) ){
                            this->imgDiffX.setValue(w, h, phaseComplement(*this->imgDiffX.getRef(w, h)));
                            this->imgDiffY.setValue(w, h, phaseComplement(*this->imgDiffY.getRef(w, h)));
                        }
                    }
                }
                this->imgCurlX.execute();
                this->imgCurlY.execute();
				for( int h = 0; h <this->height; h++){
					for( int w = 0; w <this->width; w++){
                        this->setValue( w, h, *(this->imgCurlX.getRef(w, h)) + *(this->imgCurlY.getRef(w, h) ));
                    }
                }
                return;
			};
            
		};
        
        template <class T>
        class DivPhaseSingularityAnalyzer : public ImageAnalyzer<T, unsigned char>{
        private:
            int winSize;
            T thre;
        public:
            
            explicit DivPhaseSingularityAnalyzer( MLARR::Basic::Image<T>& _src, int _winSize, T _thre)
            :ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src), winSize( _winSize ), thre(_thre) {};
            
            DivPhaseSingularityAnalyzer( MLARR::Basic::Image<T>& _src, DivPhaseSingularityAnalyzer<T>& pre)
            : ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src), winSize( pre.winSize ), thre(pre.thre){};
            
            ~DivPhaseSingularityAnalyzer(void){};
            
        public:
            void execute(void){
                
                dynamic_cast<MLARR::Basic::Image<unsigned char>*>(this)->clear(0);
				for( int h = 0; h <= this->height - winSize; h++){
					for( int w = 0; w <= this->width - winSize; w++){
                        int h_c = h + ( winSize - 1 )/2;
                        int w_c = w + ( winSize - 1 )/2;
                        
						if( *(this->im_roi.getRef(w_c, h_c)) ){
                            
                            double base = *(this->srcImg.getRef(w_c, h_c));
                            
							// evaluate div
                            double div = 0.0;
							for( int i = 0; i < winSize; i++){
								for( int j = 0; j < winSize; j++){
                                    double diff = (*(this->srcImg.getRef(w+i, h+j))) - base;
                                    diff = phaseComplement( diff );
									div += diff * diff;
								}
							}
                            div = sqrt( div ) / static_cast<double>( winSize * winSize );
                            div /= M_PI;
							this->setValue(w_c, h_c, static_cast<T>(div) > this->thre ? 1 : 0);
						}
					}
				}
            };
        };
        
        template <class T>
        class AdjPhaseSingularityAnalyzer : public ImageAnalyzer<T, unsigned char>{
        
        private:
            const int winSize = 5;

        public:
            explicit AdjPhaseSingularityAnalyzer(MLARR::Basic::Image<T>& _src )
            :ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src){};

            explicit AdjPhaseSingularityAnalyzer(MLARR::Basic::Image<T>& _src , AdjPhaseSingularityAnalyzer<T>& pre)
            :ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src){};
            
            ~AdjPhaseSingularityAnalyzer(void){};
            
        public:
            void execute(void){
                dynamic_cast<MLARR::Basic::Image<unsigned char>*>(this)->clear(0);
				for( int h = 0; h <= this->height - winSize; h++){
					for( int w = 0; w <= this->width - winSize; w++){
                        int h_c = h + ( winSize - 1 )/2;
                        int w_c = w + ( winSize - 1 )/2;
                        if( *(this->im_roi.getRef(w_c, h_c)) ){
                            
                            double base = *(this->srcImg.getRef(w_c, h_c));
                            int flag[4] = {0,0,0,0};
                            
							for( int i = 0; i < winSize; i++){
								for( int j = 0; j < winSize; j++){
                                    double diff = (*(this->srcImg.getRef(w+i, h+j))) - base;
                                    diff = phaseComplement( diff ) + M_PI;
                                    flag[ static_cast<int>( diff / (M_PI / 2) )] = 1;
                                    //flag[ static_cast<int>( (*(this->srcImg.getRef(w+i, h+j)) + M_PI) / (M_PI / 2) )] = 1;
								}
							}
                            if( flag[0] + flag[1] + flag[2] + flag[3] >= 3 ){
                                this->setValue(w_c, h_c, 1);
                            }
                        }
                    }
                }
            };
        };
		
        
    }
}

#endif
