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
        
		template<class T_IN, class T_OUT>
		class ImageAnalyzer : public MLARR::Basic::Image<T_OUT>
		{
		protected:
			MLARR::Basic::Image<T_IN>& srcImg;
			MLARR::Basic::Image<unsigned char> im_roi;
            MLARR::Basic::Image<unsigned char> im_bin;
		public:
			explicit ImageAnalyzer( const int _height, const int _width, const T_OUT& iniValue, MLARR::Basic::Image<T_IN>& _srcImg )
				: MLARR::Basic::Image<T_OUT>(_height, _width, iniValue), im_roi( _height, _width, 1 ), im_bin( _height, _width, 1 ),srcImg( _srcImg ){
			};
			virtual ~ImageAnalyzer(){};
			MLARR::Basic::Image<unsigned char>& getRoi(void){ return this->im_roi; };
			void setRoi(const MLARR::Basic::Image<unsigned char>& src){ this->im_roi = src; };
			virtual void execute(void) = 0; // implement analysis of srcImg.
            MLARR::Basic::Image<unsigned char>& getBin(void){
				for( int h = 0; h < this->height ; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.getRef(w, h)) && *(this->srcImg.getRef(w, h)) > 0 ){
                            this->im_bin.setValue(w, h, 1);
                        }else{
                            this->im_bin.setValue(w, h, 0);
                        }
                    }
                }
                return this->im_bin;
            };
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
        class MorphImage : public ImageAnalyzer<T, T>
        {
        protected:
            int step;
        public:
            explicit MorphImage(MLARR::Analyzer::Image<T>& _srcImg, int _step)
            : ImageAnalyzer<T,T>(_srcImg.height , _srcImg.width, 0, _srcImg ), step(_step){
            };
            ~MorphImage(void){};
            void execute(void){
                cv::Mat* temp = this->srcImg.clone();
                cv::Mat dst(*temp);
                if(step>0){
                    cv::dilate( *temp, dst, cv::Mat(), cv::Point(-1,-1), step );
                }else{
                    cv::erode( *temp, dst, cv::Mat(), cv::Point(-1,-1), -step );
                }
                *dynamic_cast<MLARR::Basic::Image<T>*>(this) = dst;
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
        
        class LabelImage : public ImageAnalyzer<unsigned char, unsigned char>{
        protected:
            std::map<int, int> map_LUT;
            std::map<int, std::vector< MLARR::Basic::Point<int> > > map_cluster;
        public:
            std::vector< MLARR::Basic::Point<double> > vec_ps;
        public:
            explicit LabelImage(MLARR::Basic::Image<unsigned char>& _srcImg)
            : ImageAnalyzer<unsigned char, unsigned char>( _srcImg.height, _srcImg.width, 0, _srcImg){};
            virtual ~LabelImage(void){};
            void execute(void){
                
                dynamic_cast<MLARR::Basic::Image<unsigned char>*>(this)->clear(0);
                map_LUT.clear();
                map_cluster.clear();
                vec_ps.clear();
                
                int label = 1;
				for( int h = 1; h < this->height - 1; h++){
					for( int w = 1; w < this->width - 1; w++){
                        if( *(this->srcImg.getRef(w, h))){
                            int sum = 0;
                            int tempLabel = -1;
                            unsigned char v1 = *(this->getRef(w-1, h));
                            unsigned char v2 = *(this->getRef(w+1, h-1));
                            unsigned char v3 = *(this->getRef(w,   h-1));
                            unsigned char v4 = *(this->getRef(w-1, h-1));
                            sum = v1 + v2 + v3 + v4;
                            if( sum == 0 ){
                                tempLabel = ++label;
                                map_LUT[tempLabel] = tempLabel;
                            }else{
                                int min = INT_MAX;
                                if( v1 ) min = v1 < min ? v1 : min;
                                if( v2 ) min = v2 < min ? v2 : min;
                                if( v3 ) min = v3 < min ? v3 : min;
                                if( v4 ) min = v4 < min ? v4 : min;
                                tempLabel = min;
                                if( v1 && min != v1 ) map_LUT[v1] = min;
                                if( v2 && min != v2 ) map_LUT[v2] = min;
                                if( v3 && min != v3 ) map_LUT[v3] = min;
                                if( v4 && min != v4 ) map_LUT[v4] = min;
                            }
                            this->setValue(w, h, tempLabel);
                        }
                    }
                }
                
                for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
                        if( *(this->getRef(w, h))){
                            int tempLabel = map_LUT[*(this->getRef(w, h))];
                            this->setValue(w, h, tempLabel );
                            map_cluster[tempLabel].push_back(MLARR::Basic::Point<int>(w, h));
                        }
                    }
                }
                
                std::map<int, std::vector<MLARR::Basic::Point<int> > >::iterator it;
                for( it = this->map_cluster.begin(); it != map_cluster.end(); it++){
                    MLARR::Basic::Point<int> sum(0,0);
                    MLARR::Basic::Point<double> mean(0,0);
                    std::vector<MLARR::Basic::Point<int> >::iterator v;
                    for( v = it->second.begin(); v != it->second.end(); v++ ){
                        sum = sum + *v;
                    }
                    mean.setX( sum.getX() / static_cast<double>(it->second.size()) );
                    mean.setY( sum.getY() / static_cast<double>(it->second.size()) );
                    this->vec_ps.push_back(mean);
                }
                
            };
        };
        

        
        template< typename T_IN, class SHRINKER, class ANALYZER>
        class PyramidDetector : public ImageAnalyzer< T_IN, unsigned char >{
        public:
            int pyrNum;
            std::vector< SHRINKER* > vec_shrinker;
            std::vector< ANALYZER* > vec_analyzer;
        protected:
        public:
            explicit PyramidDetector(MLARR::Basic::Image<T_IN>& _src, ANALYZER* root, int _pyrNum)
            : ImageAnalyzer<T_IN, unsigned char>( _src.height, _src.width, 0, _src ), pyrNum(_pyrNum){
                int c = 0;
                vec_analyzer.push_back(root);
                while( c++ < pyrNum - 1 ){
                    if( vec_shrinker.size() ){
                        vec_shrinker.push_back( new SHRINKER(*(dynamic_cast<MLARR::Basic::Image<T_IN>*>(vec_shrinker.back()))));
                    }else{
                        vec_shrinker.push_back( new SHRINKER(_src) );
                    }
                    vec_analyzer.push_back(
                           new ANALYZER(
                                *dynamic_cast<MLARR::Basic::Image<T_IN>*>(vec_shrinker.back()),
                                *dynamic_cast<ANALYZER*>(vec_analyzer.back())));
                }
            };
            
            virtual ~PyramidDetector(void){
                for( int i = 0; i < vec_shrinker.size(); i++){
                    delete vec_shrinker[i];
                }
                for( int i = 1; i < vec_analyzer.size(); i++){
                    /* first analyzer should be deleted externally */
                    delete vec_analyzer[i];
                }
            };
            
            void setRoi( const MLARR::Basic::Image<unsigned char>& _roi ){
                vec_analyzer[0]->setRoi( _roi );
                for( int i = 1; i < vec_analyzer.size(); i++){
                    ImageShrinker<unsigned char> imgHalf( vec_analyzer[ i-1 ]->getRoi() );
                    imgHalf.execute();
                    vec_analyzer[i]->setRoi( imgHalf );
                }
                
            }
            
            void execute(void){
                using namespace MLARR;
                
                /* cascade execution */
                for( int i = 0; i < vec_shrinker.size(); i++){
                    vec_shrinker[i]->execute();
                }
                for( int i = static_cast<int>(vec_analyzer.size()) - 1; i > 0 ; i--){
                    vec_analyzer[i]->execute();
                    MorphImage<unsigned char> morph(vec_analyzer[i]->getBin(), 1);
                    morph.execute();
                    vec_analyzer[i-1]->setRoi( morph );
                }
                vec_analyzer[0]->execute();
                
                *dynamic_cast<Image<unsigned char>*>(this) = *dynamic_cast<Image<unsigned char>*>(vec_analyzer[0]);
                
                /* merge binary results */
                /*
                this->im_bin = vec_analyzer[0]->getBin();
                for( int i = 1; i < vec_analyzer.size(); i++){
                    int rate = pow(2,i);
                    ANALYZER* ptr = vec_analyzer[i];
                    for( int h = 0; h < ptr->height; h++){
                        for( int w = 0; w < ptr->width; w++){
                            int sum = 0;
                            if( ptr->getBin().getRef(w,h) ){
                                for( int n = 0; n < rate; n++){
                                    for( int m = 0; m < rate; m++){
                                        sum += static_cast<int>(*this->im_bin.getRef(w+m, h+n));
                                    }
                                }
                                if( sum == 0 ){
                                    for( int n = 0; n < rate; n++){
                                        for( int m = 0; m < rate; m++){
                                            this->im_bin.setValue(w+m, h+n, 1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }*/
                
            };
        };
        
	}

}