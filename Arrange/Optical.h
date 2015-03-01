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
#include <math.h>
#include <float.h>
#include <algorithm>
#include <vector>

#include "IO.h"
#include "Analyzer.h"
#include "VectorAnalyzer.h"

using namespace MLARR::Basic;

namespace MLARR{
    
	namespace Analyzer{
        
        /* convert phase value to be in range from -PI to PI */
        inline double phaseComplement(double x){
            int pos = floor( ( x + M_PI ) / (2 * M_PI) );
            return x - pos * 2 * M_PI;
        };
        
        inline double phaseAbsDiff(double x, double y){
            double diff = abs(x - y);
            diff = phaseComplement(diff - M_PI ) + M_PI;
            diff = diff < ( 2 * M_PI - diff ) ? diff : ( 2 * M_PI - diff );
            return diff;
        };
        
        inline double phaseDiv( std::vector<double> &vec, int maxCount ){
            double div = 0.0;
            double ave_cos = 0.0;
            double ave_sin = 0.0;
            for( std::vector<double>::iterator it = vec.begin(); it != vec.end(); it++){
                ave_cos += cos( *it );
                ave_sin += sin( *it );
            }
            if( vec.size() ){
                ave_cos /= (double)vec.size();
                ave_sin /= (double)vec.size();
            }
            div = 1.0 - sqrt( ave_cos * ave_cos + ave_sin * ave_sin );
            if(maxCount)
                div *= vec.size() / static_cast<double>(maxCount);
            return div;
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
						if( *(this->im_roi.at(w, h)) ){
							T max = *(this->maxImage.at(w, h));
							T range = *(this->im_range.at(w, h));
							T val = *(this->srcImg.at(w, h));
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
						T range = *(maxImage.at(w, h)) - *(minImage.at(w, h));
						this->im_range.setValue( w, h, range);
						this->im_roi.setValue( w, h, range > roiThre ? 1 : 0 );
					}
				}
			};
		};
        
        template <typename T, class FIL>
        class ActivationTimeMap : public ImageAnalyzer<T, unsigned short>
        {
        protected:
            int minPeakDistance;
            Image< FIL > imgFilter;
        public:
            explicit ActivationTimeMap( Image<T>& _srcImg, FIL &_fil, int _minPeakDistance )
            : ImageAnalyzer<T, unsigned short>( _srcImg.height, _srcImg.width, 0, _srcImg ),
              imgFilter( _srcImg.height, _srcImg.width, _fil ), minPeakDistance(_minPeakDistance)
            {};
            ~ActivationTimeMap(void){};
        public:
            void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
                        if( *(this->im_roi.at(w, h)) ){
                            FIL *ptrFil = imgFilter.at(w, h);
                            unsigned char buf = ptrFil->refValue(); /* filter output value on the last frame */
                            T val = *(this->srcImg.at(w, h));
                            ptrFil->update(val);
                            /* count up */
                            if( ptrFil->refValue() > 0 && buf == 0 && *this->at(w,h) >= minPeakDistance ){
                                this->setValue(w, h, 1); /* counter reset */
                            }else{
                                this->setValue(w, h, *(this->at(w, h)) + 1); /* increment */
                            }
                        }
                    }
                }
            };
        };
        
        template<class T>
		class PhaseMedianFilter : public ImageAnalyzer<T, T>{
		private:
			int ksize;
		public:
			explicit PhaseMedianFilter(MLARR::Basic::Image<T>& _srcImg, int _ksize)
            : ImageAnalyzer<T, T>( _srcImg.height , _srcImg.width, 0, _srcImg ), ksize(_ksize){
				if( ksize % 2 == 0) {
                    throw std::string( "invalid parameter @ PhaseMedianFilter constructor. Kernel size must be add.");
				}
			};
			~PhaseMedianFilter(void){};
			void execute(void){
				for( int h = 0; h <= this->height - ksize; h++){
					for( int w = 0; w <= this->width - ksize; w++){
						if( *(this->im_roi.at(w, h)) ){
							int h_c = h + ( ksize - 1 )/2;
							int w_c = w + ( ksize - 1 )/2;
                            T val_c = *this->srcImg.at(w_c, h_c);
							std::vector<T> v;
							for( int i = 0; i < ksize; i++){
								for( int j = 0; j < ksize; j++){
									v.push_back(phaseComplement(*(this->srcImg.at(w+i, h+j)) - val_c));
								}
							}
							std::nth_element( v.begin(), v.begin() + v.size() / 2, v.end() );
							this->setValue(w_c, h_c, phaseComplement( val_c + v[ v.size() / 2]));
						}
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
						if( *(this->im_roi.at(w_c, h_c)) ){
                            
                            // search Max, Min
                            double min = 2 * M_PI;
                            for( int i = 0; i < this->coeffImg.width; i++){
								for( int j = 0; j < this->coeffImg.height; j++){
                                    double value = *(this->srcImg.at(w+i, h+j));
                                    min = min > value ? value : min;
								}
							}
                            
							// position of center pix
							double value = 0.0;
							for( int i = 0; i < this->coeffImg.width; i++){
								for( int j = 0; j < this->coeffImg.height; j++){
                                    double diff = (*(this->srcImg.at(w+i, h+j))) - min;
									value += phaseComplement( diff ) * (*(this->coeffImg.at(i, j)));
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
							if( *(this->im_roi.at(w, h)) ){
								double val = 0.0;
								it_buf = buffer.begin();
								it_coef = coef.begin();
								for(; it_coef!=coef.end(); it_coef++){
									T pixVal = *((*it_buf).at(w, h));
									val += (*it_coef) * pixVal;
									it_buf++;
								}
								char tempSign = val > 0.0 ? 1 : 0 ;
								char lastSign = *(this->img_sign.at(w, h));
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
            int winSize;
        public:

			explicit SimplePhaseSingularityAnalyzer( MLARR::Basic::Image<T>& _srcImg, std::vector<double> _coef, int _winSize)
            : ImageAnalyzer<T, unsigned char>( _srcImg.height, _srcImg.width, 0, dynamic_cast<MLARR::Basic::Image<T>&>(_srcImg)), imgSP(_srcImg, _coef), winSize(_winSize)
            {};
            
            explicit SimplePhaseSingularityAnalyzer(MLARR::Basic::Image<T>& _srcImg, SimplePhaseSingularityAnalyzer<T>& pre)
            : ImageAnalyzer<T, unsigned char>( _srcImg.height, _srcImg.width, 0, dynamic_cast<MLARR::Basic::Image<T>&>(_srcImg)), imgSP(_srcImg, pre.imgSP.coef), winSize(pre.winSize)
            {};
            
			~SimplePhaseSingularityAnalyzer(void){};
            
            void setRoi(const MLARR::Basic::Image<unsigned char>& src){
                ImageAnalyzer<T, unsigned char>::setRoi(src);
                imgSP.setRoi(src);
            };
            
            void setWinSize(int _winSize) { winSize = _winSize; };
			
            void execute(void){
				this->clear(0);
                imgSP.execute();
				for( int h = 0; h <= this->height - winSize; h++){
					for( int w = 0; w <= this->width - winSize; w++){
						if( *(this->im_roi.at(w, h)) ){
							int flag_on[5] = {1,1,1,1,1};
							int flag[5] = {1,0,0,0,0};
							for(int i = 0; i < winSize ; i++){
								for(int j = 0; j < winSize; j++){
									char psVal = *this->imgSP.at(w+i, h+j);
									if( psVal >= 0 && psVal <= 4){
										flag[psVal] = 1;
									}
								}
							}
							if( 0 == memcmp( flag_on , flag , sizeof(int) * 5 )){
								this->setValue( w + winSize / 2, h + winSize / 2, 1 );
							}
						}
					}
				}
			};
		};
        
        template <class T>
		class BrayPhaseSingularityAnalyzer : public ImageAnalyzer<T,unsigned char>{
        public:
            double thre;
            ImageDiffX<T> imgDiffX;
            ImageDiffY<T> imgDiffY;
            SpacialFilter<T> imgCurlX;
            SpacialFilter<T> imgCurlY;
		public:
			explicit BrayPhaseSingularityAnalyzer( MLARR::Basic::Image<double>& _src, double _thre )
			:ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src),
            thre( _thre ),
            imgDiffX( this->srcImg ), imgDiffY( this->srcImg ),
            imgCurlX( this->imgDiffX, 3, 3, MLARR::Analyzer::coefficients.vec_nablaY ),
            imgCurlY( this->imgDiffY, 3, 3, MLARR::Analyzer::coefficients.vec_nablaX ) {
            };
			~BrayPhaseSingularityAnalyzer(void){};
		public:
			void execute(void){
                
                this->imgDiffX.execute();
                this->imgDiffY.execute();
				for( int h = 0; h <this->height; h++){
					for( int w = 0; w <this->width; w++){
                        this->imgDiffX.setValue(w, h, phaseComplement(*this->imgDiffX.at(w, h)));
                        this->imgDiffY.setValue(w, h, phaseComplement(*this->imgDiffY.at(w, h)));
                    }
                }
                this->imgCurlX.execute();
                this->imgCurlY.execute();
				for( int h = 0; h <this->height; h++){
					for( int w = 0; w <this->width; w++){
                        if( *(this->im_roi.at(w, h)) ){
                            double value = double(*(this->imgCurlX.at(w, h)) + *(this->imgCurlY.at(w, h)));
                            value = abs(value);
                            this->setValue( w, h, value > this->thre * 2 * M_PI ? 1 : 0);
                            /*
                            if( value > this->thre * 2 * M_PI ){
                                std::ofstream ofs("/Users/tomii/debug.log");
                                if(!ofs) return;
                                
                                ofs << *(this->imgCurlX.at(w, h)) << endl;
                                ofs << *(this->imgCurlY.at(w, h)) << endl;
                                ofs << "----" << std::endl;
                                for( int y = -1; y <= 2; y++){
                                    for( int x = -1; x <= 2; x++){
                                        ofs << *(this->srcImg.at(w+x, h+y)) << std::endl;
                                    }
                                }
                                ofs << "----" << std::endl;
                                for( int y = -1; y <= 1; y++){
                                    for( int x = -1; x <= 1; x++){
                                        ofs << *(this->imgDiffX.at(w+x, h+y)) << ",";
                                        ofs << *(this->imgDiffY.at(w+x, h+y)) << endl;
                                    }
                                }
                                ofs.close();
                            }*/
                        }
                    }
                }
                return;
			};
            
		};
        
        template <class T>
        class DivPhaseSingularityAnalyzer : public ImageAnalyzer<T, unsigned char>{
        public:
            int winSize;
            double thre;
            Image<double> imgDiv;
            SpacialFilter<double> imgDivFil;
        public:
            
            explicit DivPhaseSingularityAnalyzer( MLARR::Basic::Image<T>& _src, int _winSize, double _thre)
            :ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src), winSize( _winSize ), thre( _thre ),
            imgDiv(_src.height, _src.width, 0), imgDivFil(imgDiv, 5,5, coefficients.vec_gaussian_5x5){};
            
            DivPhaseSingularityAnalyzer( MLARR::Basic::Image<T>& _src, DivPhaseSingularityAnalyzer<T>& pre)
            : ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src), winSize( pre.winSize ), thre( pre.thre ),
            imgDiv(_src.height, _src.width, 0), imgDivFil(imgDiv, 5,5, coefficients.vec_gaussian_5x5){};
            
            ~DivPhaseSingularityAnalyzer(void){};
            
        public:
            void execute(void){
                
                dynamic_cast<MLARR::Basic::Image<unsigned char>*>(this)->clear(0);
                
				for( int h = 0; h <= this->height - winSize; h++){
					for( int w = 0; w <= this->width - winSize; w++){
                        int h_c = h + ( winSize - 1 )/2;
                        int w_c = w + ( winSize - 1 )/2;
                        
						if( *(this->im_roi.at(w_c, h_c)) ){
                            
							// evaluate div
                            std::vector<double> vec;
							for( int i = 0; i < winSize; i++){
								for( int j = 0; j < winSize; j++){
                                    if( *(this->im_roi.at(w+i, h+j)) ){
                                        vec.push_back( *(this->srcImg.at(w+i, h+j)) );
                                    }
								}
							}
                            double div = phaseDiv( vec, winSize*winSize );
							this->imgDiv.setValue( w_c, h_c, div );
                            this->setValue(w_c, h_c, static_cast<T>(div) > this->thre ? 1 : 0); // Binarize
						}
					}
				}
                
                //imgDivFil.execute();
                
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
                        if( *(this->im_roi.at(w_c, h_c)) ){
                            
                            double base = *(this->srcImg.at(w_c, h_c));
                            int flag[4] = {0,0,0,0};
                            
							for( int i = 0; i < winSize; i++){
								for( int j = 0; j < winSize; j++){
                                    double diff = (*(this->srcImg.at(w+i, h+j))) - base;
                                    diff = phaseComplement( diff ) + M_PI;
                                    flag[ static_cast<int>( diff / (M_PI / 2) )] = 1;
                                    //flag[ static_cast<int>( (*(this->srcImg.at(w+i, h+j)) + M_PI) / (M_PI / 2) )] = 1;
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
        
        template <class T>
        class PhaseRangeDetector : public ImageAnalyzer<T, unsigned char>
        {
        protected:
            T _mean;
            T _range;
        public:
            explicit PhaseRangeDetector( MLARR::Basic::Image<T>& _src, T mean, T range)
            :ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src), _mean(mean), _range(range)
            {};
            ~PhaseRangeDetector(void){};
        public:
            void execute(void){
                this->clear();
                for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
                        if( *(this->im_roi.at(w, h)) ){
                            double p = *(this->srcImg.at(w, h));
                            this->setValue( w, h, phaseAbsDiff(p, _mean) < _range ? 1 : 0 );
                        }
                    }
                }
            };
        };
        
        class ElectrodePhaseComplement : public ImageAnalyzer<double, double>
        {
        protected:
            const int minPixNum;
            Image<unsigned char> imgMask;
        public:
            explicit ElectrodePhaseComplement( MLARR::Basic::Image<double>& _src, const std::string& maskImagePath, int _minPixNum )
            :ImageAnalyzer<double,double>( _src.height, _src.width, 0, _src), imgMask(_src.height, _src.width, maskImagePath), minPixNum(_minPixNum)
            {
                if( this->height != imgMask.height || this->width != imgMask.width){
                    throw string("Invalid size mask image : ") + maskImagePath;
                }
            };
            ~ElectrodePhaseComplement(void){};
        public:
            void execute(void){
                this->clear();
                for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
                        if( *(this->imgMask.at(w, h)) ){
                            int winRad = 1;
                            while(1){
                                double val =  *(this->srcImg.at(w, h));
                                std::vector<double> v;
                                for( int i = -winRad; i <= winRad; i++){
                                    for( int j = -winRad; j <= winRad; j++){
                                        int x = w + i;
                                        int y = h + j;
                                        if( x >= 0 && x < this->width && y >=0 && y < this->height){
                                            if(*(this->imgMask.at(x, y)) == 0 && *( this->im_roi.at(x,y) ) != 0){
                                            //if(*(this->imgMask.at(x, y)) == 0 ){
                                                v.push_back( static_cast<double>( phaseComplement( *(this->srcImg.at(x, y)) - val )));
                                            }
                                        }
                                    }
                                }
                                if( v.size() >= minPixNum ){
                                    // select median difference
                                    std::nth_element( v.begin(), v.begin() + v.size() / 2, v.end() );
                                    *this->at(w, h) = phaseComplement( val + v[ v.size() / 2] );
                                    break;
                                }
                                ++winRad;
                                if( winRad > this->width / 4 || winRad > this->height / 4 )
                                    break;
                            }
                        }else{
                            *this->at(w, h) = *this->srcImg.at(w,h);
                        }
                    }
                }
            };
        };
        
    }
}

#endif
