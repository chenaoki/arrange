#pragma once

#include "stdafx.h"
#include "Basic.h"
#include "coefs.h"
#include <float.h>
#include <algorithm>

#include "IO.h"

namespace MLARR{

	namespace Analyzer{
        
        using namespace std;
        using namespace MLARR::Basic;
        
        inline double phaseComplement(double x){
            int pos = static_cast<int>( x / M_PI);
            return x - pos * 2 * M_PI;
        };

		template<class T_IN, class T_OUT>
		class ImageAnalyzer : public MLARR::Basic::Image<T_OUT>
		{
		protected:
			MLARR::Basic::Image<T_IN>& srcImg;
			MLARR::Basic::Image<char> im_roi;
		public:
			explicit ImageAnalyzer( const int _height, const int _width, const T_OUT& iniValue,MLARR::Basic::Image<T_IN>& _srcImg )
				: MLARR::Basic::Image<T_OUT>(_height, _width, iniValue), im_roi( _height, _width, 1 ), srcImg( _srcImg ){
			};
			virtual ~ImageAnalyzer(){};
			const MLARR::Basic::Image<char>& getRoi(void){ return this->im_roi; };
			void setRoi(const MLARR::Basic::Image<char>& src){ this->im_roi = src; };
			virtual void execute(void) = 0; // implement analysis of srcImg.
			
		};

		template<class T>
		class Plotter : public ImageAnalyzer<T, unsigned char>
		{
		public:
			double max, min, amp;
			const double h_mergin;
		public:
			Plotter( const int _height, MLARR::Basic::Image<T>& _srcImg )
				: ImageAnalyzer<T, unsigned char>(_height, _srcImg.nPix(), 1, _srcImg), max(static_cast<double>( INT_MIN )), min(static_cast<double>( INT_MAX )), amp(0), h_mergin(0.1){
			};
			virtual ~Plotter(void){};
			void execute(void){

				/* initialize */
				max = -1 * DBL_MAX;
				min = DBL_MAX;
				amp = 0.0;
				this->clear(1);

				/* search max, min */
				T* ptr = this->srcImg.getPtr();
				for( int i = 0; i< this->srcImg.nPix(); i++){
					max = max >= static_cast<double>(*ptr) ? max : *ptr;
					min = min <= static_cast<double>(*ptr) ? min : *ptr;
					ptr++;
				}
				amp = max - min;

				/* draw plot */
				if( amp != 0 ){
					ptr = this->srcImg.getPtr();
					for( int i = 0; i< this->srcImg.nPix(); i++){
						size_t h = getHeight( *ptr );
						this->setValue(i, h, 0);
						ptr++;
					}
				}else{
					for( int i = 0; i< this->srcImg.nPix(); i++){
						this->setValue( i, this->height / 2 , 0 );
					}
				}
			};
			size_t getHeight(const T& value){
				return static_cast<size_t>( this->height * ( 1 - h_mergin ) * ( max - value ) / amp + ( this->height * h_mergin / 2.0 ) );
			};
			double getHeightValue(size_t h){
				return max - ( amp * ( h / this->height - h_mergin / 2.0 ) / (1 - h_mergin) );
			};
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
		class MaxImageAnalyzer : public ImageAnalyzer<T, T>
		{
		public:
			explicit MaxImageAnalyzer( const T& iniValue,MLARR::Basic::Image<T>& _srcImg )
				: ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, iniValue, _srcImg ){

			};
			~MaxImageAnalyzer(){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						T tmpMax = *(this->getRef(w, h));
						T value = *(this->srcImg.getRef(w, h));
						this->setValue( w, h, tmpMax > value ? tmpMax : value );
					}
				}
			};
		};

		template<class T>
		class MinImageAnalyzer : public ImageAnalyzer<T, T>
		{
		public:
			explicit MinImageAnalyzer( const T& iniValue,MLARR::Basic::Image<T>& _srcImg )
				: ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, iniValue, _srcImg ){

			};
			~MinImageAnalyzer(){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						T tmpMin = *(this->getRef(w, h));
						T value = *(this->srcImg.getRef(w, h));
						this->setValue( w, h, tmpMin > value ? value : tmpMin);
					}
				}
			};
		};

		
		template<class T>
		class ImageCropper : public ImageAnalyzer<T,T>
		{
		private:
			int off_x;
			int off_y;
		public:
			explicit ImageCropper(MLARR::Basic::Image<T>& _srcImg, int _off_x, int _off_y, int _w, int _h)
				: ImageAnalyzer<T, T>( _w, _h, 0, _srcImg ) , off_x( _off_x), off_y(_off_y) {
			};
			~ImageCropper(void){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						this->setValue(w, h, *(this->srcImg.getRef( off_x + w, off_y + h)));
					}
				}
			};
		};

		template<class T>
		class ImageShrinker : public ImageAnalyzer<T,T>
		{
		public:
			explicit ImageShrinker(MLARR::Basic::Image<T>& _srcImg)
				: ImageAnalyzer<T, T>( _srcImg.height/2 , _srcImg.width/2, 0, _srcImg ){
			};
			~ImageShrinker(void){};
			void execute(void){
				cv::Mat* temp = this->srcImg.clone();
				cv::Mat dst( *temp );
				cv::pyrDown( *temp, dst );
				*dynamic_cast<MLARR::Basic::Image<T>*>(this) = dst;
				delete temp;
			};
		};

		template<class T>
		class ImageThinOut : public ImageAnalyzer<T,T>
		{
		public:
			explicit ImageThinOut(MLARR::Basic::Image<T>& _srcImg)
				: ImageAnalyzer<T, T>( _srcImg.height/2 , _srcImg.width/2, 0, _srcImg ){
			};
			~ImageThinOut(void){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						this->setValue(w, h, *(this->srcImg.getRef(w*2,h*2)));
					}
				}
			};
		};

		template<class T>
		class ImageDoubler : public ImageAnalyzer<T,T>
		{
		public:
			explicit ImageDoubler(MLARR::Basic::Image<T>& _srcImg)
				: ImageAnalyzer<T, T>( _srcImg.height*2 , _srcImg.width*2, 0, _srcImg ){
			};
			~ImageDoubler(void){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						this->setValue(w, h, *(this->srcImg.getRef(w/2,h/2)));
					}
				}
			};
		};
			

		template<typename T>
		class SimplePhaseAnalyzer : public ImageAnalyzer<T, char>{
		public:
			std::vector<double> coef;
			std::vector<MLARR::Basic::Image<T>> buffer;
			MLARR::Basic::Image<char> img_sign;
			enum EPhase{unknown = 0, downstroke, peak, upstroke, bottom};
		public:
			explicit SimplePhaseAnalyzer(MLARR::Basic::Image<T>& _srcImg, std::vector<double> _coef)
				: ImageAnalyzer<T, char>(_srcImg.height, _srcImg.width, unknown, _srcImg), coef(_coef), buffer(), img_sign(_srcImg.height, _srcImg.width, 0){
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
		class SimplePhaseSingularityAnalyzer : public ImageAnalyzer<char, char>{
		public:
			explicit SimplePhaseSingularityAnalyzer( SimplePhaseAnalyzer<T>& _srcImg)
				: ImageAnalyzer<char, char>( _srcImg.height, _srcImg.width, 0, dynamic_cast<MLARR::Basic::Image<char>&>(_srcImg)){
			};
			~SimplePhaseSingularityAnalyzer(void){};
			void execute(void){
				this->clear(0);
				for( int h = 1; h < this->height - 1; h++){
					for( int w = 1; w < this->width - 1; w++){
						if( *(this->im_roi.getRef(w, h)) ){
							int flag_on[5] = {1,1,1,1,1};
							int flag[5] = {1,0,0,0,0};
							for(int i = -1; i <=1 ; i++){
								for(int j = -1; j <=1; j++){
									char psVal = *this->srcImg.getRef(w+i, h+j);
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
        class ImageDiffX : public ImageAnalyzer<T,T>{
        public:
            explicit ImageDiffX( MLARR::Basic::Image<double>& _src )
			:ImageAnalyzer<double,double>( _src.height, _src.width, 0, _src){};
            ~ImageDiffX(void){};
        public:
            void execute(void){
				this->clear(0);
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width - 1; w++){
						if( *(this->im_roi.getRef(w, h)) ){
                            double p = *this->srcImg.getRef(w, h);
                            double q = *this->srcImg.getRef(w+1, h);
                            this->setValue(w, h, q - p);
                        }
                    }
                }
            };
        };
        
        template <class T>
        class ImageDiffY : public ImageAnalyzer<T,T>{
        public:
            explicit ImageDiffY( MLARR::Basic::Image<double>& _src )
			:ImageAnalyzer<double,double>( _src.height, _src.width, 0, _src){};
            ~ImageDiffY(void){};
        public:
            void execute(void){
				this->clear(0);
				for( int h = 0; h < this->height - 1; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.getRef(w, h)) ){
                            double p = *this->srcImg.getRef(w, h);
                            double q = *this->srcImg.getRef(w, h+1);
                            this->setValue(w, h, q - p);
                        }
                    }
                }
            };
        };

        
		template <class T>
		class BinaryOr : public ImageAnalyzer<T, T>{
		private:
			const MLARR::Basic::Image<T>& cmpImg;
			const T thre;
		public:
			explicit BinaryOr( MLARR::Basic::Image<T>& _srcImg, const MLARR::Basic::Image<T>& _cmpImg, T _thre = 0 )
				: ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg), cmpImg(_cmpImg), thre(_thre){
					if( _srcImg.width != _cmpImg || _srcImg.height != _cmpImg.height ){
						throw "error : comparison image have invalid size.";
					}
			};
			~BinaryOr(void){};
			void execute(void){
				for( int h = 0; h < this->height ; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.getRef(w, h)) ){
							T valSrc = *(this->srcImg.getRef(w, h ));
							T valCmp = *(this->cmpImg.getRef(w, h ));
							this->setValue(w, h, ( valSrc > thre || valCmp > thre ) ? 1 : 0 );
						}
					}
				}
			};
		};

		template <class T>
		class BinaryAnd : public ImageAnalyzer<T, T>{
		private:
			const MLARR::Basic::Image<T>& cmpImg;
			const T thre;
		public:
			explicit BinaryAnd( MLARR::Basic::Image<T>& _srcImg, const MLARR::Basic::Image<T>& _cmpImg, T _thre = 0 )
				: ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg), cmpImg(_cmpImg), thre(_thre){
					if( _srcImg.width != _cmpImg.width || _srcImg.height != _cmpImg.height ){
						throw "error : comparison image have invalid size.";
					}
			};
			~BinaryAnd(void){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.getRef(w, h)) ){
							T valSrc = *(this->srcImg.getRef(w, h ));
							T valCmp = *(this->cmpImg.getRef(w, h ));
							this->setValue(w, h, ( valSrc > thre && valCmp > thre ) ? 1 : 0 );
						}
					}
				}
			};
		};

		template<class T>
		class SpacialFilter : public ImageAnalyzer<T, T>{
		protected:
			const MLARR::Basic::Image<double> coeffImg;
		public:
			explicit SpacialFilter( MLARR::Basic::Image<T>& _srcImg, const MLARR::Basic::Image<double>& _coeffImg)
				: ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg), coeffImg(_coeffImg){
			};
			explicit SpacialFilter( MLARR::Basic::Image<T>& _srcImg, int filWidth, int filHeight, const std::vector<double>& _coeff)
            : ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg), coeffImg(_coeff, filHeight, filWidth){
			};
			virtual ~SpacialFilter(void){};
			void execute(void){
				*(dynamic_cast<MLARR::Basic::Image<T>*>(this)) = this->srcImg;
				for( int h = 0; h <= this->height - coeffImg.height; h++){
					for( int w = 0; w <= this->width - coeffImg.width; w++){
                        int h_c = h + ( coeffImg.height - 1 )/2;
                        int w_c = w + ( coeffImg.width - 1 )/2;
						if( *(this->im_roi.getRef(w_c, h_c)) ){
							// position of center pix
							double value = 0.0;
							for( int i = 0; i < coeffImg.width; i++){
								for( int j = 0; j < coeffImg.height; j++){
									value += static_cast<double>( (*(this->srcImg.getRef(w+i, h+j))) * (*(coeffImg.getRef(i, j))));
								}
							}
							this->setValue(w_c, h_c, static_cast<T>(value));
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
		

		template<class T>
		class MedianFilter : public ImageAnalyzer<T, T>{
		private:
			int ksize;
		public:
			explicit MedianFilter(MLARR::Basic::Image<T>& _srcImg, int _ksize)
				: ImageAnalyzer<T, T>( _srcImg.height , _srcImg.width, 0, _srcImg ), ksize(_ksize){
				if( ksize % 2 == 0) {
					throw "invalid parameter @ MedianFilter constructor.";
				}
			};
			~MedianFilter(void){};
			void execute(void){
				for( int h = 0; h <= this->height - ksize; h++){
					for( int w = 0; w <= this->width - ksize; w++){
						if( *(this->im_roi.getRef(w, h)) ){
							int h_c = h + ( ksize - 1 )/2;
							int w_c = w + ( ksize - 1 )/2;
							std::vector<T> v;
							for( int i = 0; i < ksize; i++){
								for( int j = 0; j < ksize; j++){
									v.push_back(*(this->srcImg.getRef(w+i, h+j)));
								}
							}
							std::nth_element( v.begin(), v.begin() + v.size() / 2, v.end() );
							this->setValue(w_c, h_c, v[ v.size() / 2]);
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
        class DivPhaseSingularityAnalyzer : public ImageAnalyzer<T, T>{
        private:
            int winSize;
        public:
            explicit DivPhaseSingularityAnalyzer( MLARR::Basic::Image<T> _src, int _winSize)
            :ImageAnalyzer<double,double>( _src.height, _src.width, 0, _src), winSize( _winSize ){};
            ~DivPhaseSingularityAnalyzer(void){};
        public:
            void execute(void){
                
                dynamic_cast<MLARR::Basic::Image<T>*>(this)->clear(0);
				for( int h = 0; h <= this->height - winSize; h++){
					for( int w = 0; w <= this->width - winSize; w++){
                        int h_c = h + ( winSize - 1 )/2;
                        int w_c = w + ( winSize - 1 )/2;

						if( *(this->im_roi.getRef(w_c, h_c)) ){
                            
                            double base = 0.0;
                            for( int i = 0; i < winSize; i++){
                                for( int j = 0; j < winSize; j++){
                                    base += *(this->srcImg.getRef(w+i, h+j));
                                }
                            }
                            base /= static_cast<double>( winSize * winSize );
                            
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
                            div /= 2 * M_PI;
							this->setValue(w_c, h_c, static_cast<T>(div));
						}
					}
				}
            };
            
        };
        
	}

}