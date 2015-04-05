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
						if( *(this->im_roi.at(w, h)) && *(this->at(w, h)) > 0 ){
                            this->im_bin.setValue(w, h, 1);
                        }else{
                            this->im_bin.setValue(w, h, 0);
                        }
                    }
                }
                return this->im_bin;
            };
		};
        
        template<class T_IN, class T_OUT>
		class MovieAnalyzer : public Image<T_OUT>{
		protected:
            MLARR::Basic::Image<T_IN>& srcImg;
			MLARR::Basic::Image<unsigned char> im_roi;
			std::vector< Image<T_IN> > srcImgBuf;
			std::vector< Image<T_OUT> > outImgBuf;
			int nBufSrc;
			int nBufOut;
			int nOutPos;
			int flagValid;
		public:
			MovieAnalyzer( Image<T_IN>& _srcImg, int _nBufSrc, int _nBufOut, int _nOutPos )
            : Image<T_OUT>( _srcImg ), srcImg(_srcImg),
            im_roi(_srcImg.height, _srcImg.width, 1),
            srcImgBuf(), outImgBuf(), nBufOut(_nBufOut), nBufSrc(_nBufSrc), nOutPos(_nOutPos), flagValid(0){
			};
			virtual ~MovieAnalyzer(void){};
			const Image<char>& getRoi(void){ return im_roi; };
			void setRoi(const Image<unsigned char>& src){ im_roi = src; };
			void updateSrc(void){
				this->srcImgBuf.push_back( this->srcImg );
				while( this->srcImgBuf.size() > static_cast<size_t>(nBufSrc) ){
					this->srcImgBuf.erase( this->srcImgBuf.begin());
				}
			};
			virtual void execute(void) = 0;
            
		};
        
        template<typename TIN, class TOUT>
        class TimeSeriesFilter{
        protected:
            std::vector<TIN> buffer;
            TOUT ret;
        public:
            TimeSeriesFilter( void ) : buffer(), ret(0) {};
            TimeSeriesFilter( TOUT iniValue ) : buffer(), ret(iniValue){};
            TimeSeriesFilter( const TimeSeriesFilter<TIN, TOUT>& rhs ) : buffer(), ret(rhs.ret) {};
            virtual ~TimeSeriesFilter(){};
            TimeSeriesFilter& operator=( const TimeSeriesFilter<TIN, TOUT>& rhs ){
                this->ret = rhs.ret;
                this->buffer.clear();
                std::copy( rhs.buffer.begin(), rhs.buffer.end(), std::back_inserter(this->buffer));
            };
            TOUT& refValue(void){ return ret;};
            virtual void update(TIN& temp){};
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
						T tmpMax = *(this->at(w, h));
						T value = *(this->srcImg.at(w, h));
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
						T tmpMin = *(this->at(w, h));
						T value = *(this->srcImg.at(w, h));
						this->setValue( w, h, tmpMin > value ? value : tmpMin);
					}
				}
			};
		};
        
        template <typename T>
        class RangeDetector : public ImageAnalyzer<T, unsigned char>
        {
        protected:
            T _min;
            T _max;
        public:
            explicit RangeDetector( MLARR::Basic::Image<T>& _src, T min, T max)
            :ImageAnalyzer<T,unsigned char>( _src.height, _src.width, 0, _src), _min(min), _max(max)
            {};
            ~RangeDetector(void){};
        public:
            void setRange( T min, T max){
                _min = min;
                _max = max;
                return;
            };
            void execute(void){
                this->clear();
                for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
                        if( *(this->im_roi.at(w, h)) ){
                            T val = *(this->srcImg.at(w, h));
                            // this->setValue( w, h, val <= _max && val >= _min ? 1 : 0 );
                            if( val <= _max && val >= _min ){
                                this->setValue( w, h, 1);
                            }
                        }
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
						this->setValue(w, h, *(this->srcImg.at( off_x + w, off_y + h)));
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
						this->setValue(w, h, *(this->srcImg.at(w*2,h*2)));
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
						this->setValue(w, h, *(this->srcImg.at(w/2,h/2)));
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
						if( *(this->im_roi.at(w, h)) ){
                            double p = *this->srcImg.at(w, h);
                            double q = *this->srcImg.at(w+1, h);
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
						if( *(this->im_roi.at(w, h)) ){
                            double p = *this->srcImg.at(w, h);
                            double q = *this->srcImg.at(w, h+1);
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
                        throw std::string("error : comparison image have invalid size.");
					}
			};
			~BinaryOr(void){};
			void execute(void){
				for( int h = 0; h < this->height ; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.at(w, h)) ){
							T valSrc = *(this->srcImg.at(w, h ));
							T valCmp = *(this->cmpImg.at(w, h ));
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
						throw std::string("error : comparison image have invalid size.");
					}
			};
			~BinaryAnd(void){};
			void execute(void){
				for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
						if( *(this->im_roi.at(w, h)) ){
							T valSrc = *(this->srcImg.at(w, h ));
							T valCmp = *(this->cmpImg.at(w, h ));
							this->setValue(w, h, ( valSrc > thre && valCmp > thre ) ? 1 : 0 );
						}
					}
				}
			};
		};
        
        template <class T>
		class BinaryAdjacent : public ImageAnalyzer<T, T>{
		private:
			const MLARR::Basic::Image<T>& cmpImg;
			const T thre;
            const int winSize;
		public:
			explicit BinaryAdjacent( MLARR::Basic::Image<T>& _srcImg, const MLARR::Basic::Image<T>& _cmpImg, int _winSize, T _thre = 0 )
            : ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg), cmpImg(_cmpImg), winSize(_winSize), thre(_thre){
                if( _srcImg.width != _cmpImg.width || _srcImg.height != _cmpImg.height ){
                    throw string("error : comparison image have invalid size.");
                }
                if( winSize <= 0 ){
                    throw string("error : invalid winsize for BinaryAdjacent");
                }
			};
			~BinaryAdjacent(void){};
			void execute(void){
                this->clear(0);
				for( int h = 0; h <= this->height - winSize; h++){
					for( int w = 0; w <= this->width - winSize; w++){
                        int h_c = h + ( winSize - 1 )/2;
                        int w_c = w + ( winSize - 1 )/2;
						if( *(this->im_roi.at(w_c, h_c)) ){
                            int cntSrc = 0;
                            int cntCmp = 0;
                            for( int i = 0; i < winSize; i++){
								for( int j = 0; j < winSize; j++){
									cntSrc += *(this->srcImg.at(w + i, h + j)) > thre ? 1 : 0;
									cntCmp += *(this->cmpImg.at(w + i, h + j)) > thre ? 1 : 0;
                                }
							}
                            this->setValue(w_c, h_c, ( cntSrc && cntCmp ) ? 1 : 0);
                        }
					}
				}
			};
		};
        
        template <class T>
		class BinaryThinLine : public ImageAnalyzer<T, T>{ /* Tamura's algorhythm. */
        private:
            static inline bool isPattern1(const std::vector<T>& vec){
                return ( vec[1] == 0 || vec[5] == 0 );
            };
            static inline bool isPattern2(const std::vector<T>& vec){
                return ( vec[7] == 0 || vec[3] == 0 );
            };
            static inline bool isExcept_common(const std::vector<T>& vec){
                return (( vec[3] == 0 && vec[5] == 0 && vec[7] == 1) ||
                        ( vec[1] == 0 && vec[3] == 1 && vec[7] == 0) ||
                        ( vec[1] == 1 && vec[3] == 0 && vec[5] == 0) ||
                        ( vec[1] == 0 && vec[5] == 1 && vec[7] == 0) ||
                        ( vec[3] == 0 && vec[6] == 1 && vec[7] == 0) ||
                        ( vec[0] == 1 && vec[1] == 0 && vec[3] == 0) ||
                        ( vec[1] == 0 && vec[2] == 1 && vec[5] == 0) ||
                        ( vec[5] == 0 && vec[7] == 0 && vec[8] == 1) ||
                        ( vec[0] == 0 && vec[1] == 1 && vec[2] == 0 && vec[3] == 1 && vec[5] == 1 && vec[6] == 0 && vec[8] == 0) ||
                        ( vec[0] == 0 && vec[1] == 1 && vec[2] == 0 && vec[5] == 1 && vec[6] == 0 && vec[7] == 1 && vec[8] == 0) ||
                        ( vec[0] == 0 && vec[2] == 0 && vec[3] == 1 && vec[5] == 1 && vec[6] == 0 && vec[7] == 1 && vec[8] == 0) ||
                        ( vec[0] == 0 && vec[1] == 1 && vec[2] == 0 && vec[3] == 1 && vec[6] == 0 && vec[7] == 1 && vec[8] == 0));
            };
            static inline bool isExcept1(const std::vector<T>& vec){
                if( isExcept_common(vec) ){
                    return true;
                }else{
                    return (( vec[1] == 0 && vec[5] == 1 && vec[7] == 1 && vec[8] == 0) ||
                            ( vec[0] == 0 && vec[1] == 1 && vec[3] == 1 && vec[5] == 0 ));
                }
            }
            static inline bool isExcept2(const std::vector<T>& vec){
                if( isExcept_common(vec) ){
                    return true;
                }else{
                    return (( vec[0] == 0 && vec[1] == 1 && vec[3] == 1 && vec[7] == 0) ||
                            ( vec[3] == 0 && vec[5] == 1 && vec[7] == 1 && vec[8] == 0 ));
                }
            }
            inline void get9box( int w_c, int h_c, std::vector<T>& vec ){
                vec.clear();
                if( w_c > 0 && w_c < this->width - 1 && h_c > 0 && w_c < this->height - 1 ){
                    for( int i = -1; i <= 1; i++){
                        for( int j = -1; j <= 1; j++){
                            vec.push_back( *(this->at(w_c + j, h_c + i)) );
                        }
                    }
                }
            };
		public:
			explicit BinaryThinLine( MLARR::Basic::Image<T>& _srcImg )
            : ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg){
			};
			~BinaryThinLine(void){};
			void execute(void){
                
                dynamic_cast< Image<T>& > (*this) = this->srcImg;
                Image<T> imgTemp(*this);
                
                //IO::Display<unsigned char> dispSrc("src", this->srcImg, 1, 0, IO::colMap_gray);
                //IO::Display<unsigned char> dispTmp("tmp", imgTemp, 1, 0, IO::colMap_gray);
                //dispSrc.show();
                
                while(1){
                    
                    bool flg = false;
                    std::vector<T> vec;
                    
                    /* Pattern 1 check. */
                    imgTemp.clear(0);
                    for( int h = 0; h <= this->height - 3; h++){
                        for( int w = 0; w <= this->width - 3; w++){
                            int h_c = h + 1;
                            int w_c = w + 1;
                            T val = 0;
                            if( *(this->im_roi.at(w_c, h_c)) ){
                                val = *this->at(w_c, h_c);
                                if( val == 1 ){
                                    get9box(w_c, h_c, vec);
                                    if( 9 == vec.size() ){
                                        if( isPattern1(vec) ){
                                            if( !isExcept1(vec) ){
                                                flg = true;
                                                val = 0;
                                            }
                                        }
                                    }
                                }
                            }
                            *imgTemp.at(w_c, h_c) = val;
                        }
                    }
                    //dispTmp.show();
                    //cvWaitKey(-1);
                    dynamic_cast< Image<T>& >(*this) = imgTemp;
                    if(!flg) break;
                    
                    /* Pattern 2 check. */
                    imgTemp.clear(0);
                    for( int h = 0; h <= this->height - 3; h++){
                        for( int w = 0; w <= this->width - 3; w++){
                            int h_c = h + 1;
                            int w_c = w + 1;
                            T val = 0;
                            if( *(this->im_roi.at(w_c, h_c)) ){
                                val = *this->at(w_c, h_c);
                                if( val == 1 ){
                                    get9box(w_c, h_c, vec);
                                    if( 9 == vec.size() ){
                                        if( isPattern2(vec) ){
                                            if( !isExcept1(vec) ){
                                                flg = true;
                                                val = 0;
                                            }
                                        }
                                    }
                                }
                            }
                            *imgTemp.at(w_c, h_c) = val;
                        }
                    }
                    //dispTmp.show();
                    //cvWaitKey(-1);
                    dynamic_cast< Image<T>& >(*this) = imgTemp;
                    if(!flg) break;
                    
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
						if( *(this->im_roi.at(w_c, h_c)) ){
							// position of center pix
							double value = 0.0;
							for( int i = 0; i < coeffImg.width; i++){
								for( int j = 0; j < coeffImg.height; j++){
									value += static_cast<double>( (*(this->srcImg.at(w+i, h+j))) * (*(coeffImg.at(i, j))));
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
                    throw std::string("invalid parameter @ MedianFilter constructor. kernel size must be add.");
				}
			};
			~MedianFilter(void){};
			void execute(void){
				for( int h = 0; h <= this->height - ksize; h++){
					for( int w = 0; w <= this->width - ksize; w++){
						if( *(this->im_roi.at(w, h)) ){
							int h_c = h + ( ksize - 1 )/2;
							int w_c = w + ( ksize - 1 )/2;
							std::vector<T> v;
							for( int i = 0; i < ksize; i++){
								for( int j = 0; j < ksize; j++){
									v.push_back(*(this->srcImg.at(w+i, h+j)));
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
                        if( *(this->srcImg.at(w, h)) > 0 ){
                            int tempLabel;
                            unsigned char v1 = *(this->at(w-1, h));
                            unsigned char v2 = *(this->at(w+1, h-1));
                            unsigned char v3 = *(this->at(w,   h-1));
                            unsigned char v4 = *(this->at(w-1, h-1));
                            if( v1 == 0 && v2 == 0 && v3 == 0 && v4 == 0 ){
                                tempLabel = label++;
                                map_LUT[tempLabel] = tempLabel;
                            }else{
                                int min = INT_MAX;
                                if( v1 > 0 ) min = v1 < min ? v1 : min;
                                if( v2 > 0 ) min = v2 < min ? v2 : min;
                                if( v3 > 0 ) min = v3 < min ? v3 : min;
                                if( v4 > 0 ) min = v4 < min ? v4 : min;
                                tempLabel = min;
                                if( v1 > 0 && min != v1 ) map_LUT[v1] = min;
                                if( v2 > 0 && min != v2 ) map_LUT[v2] = min;
                                if( v3 > 0 && min != v3 ) map_LUT[v3] = min;
                                if( v4 > 0 && min != v4 ) map_LUT[v4] = min;
                            }
                            this->setValue(w, h, tempLabel);
                        }
                    }
                }
                
                for( std::map<int,int>::iterator it = map_LUT.begin(); it != map_LUT.end(); it++){
                    it->second = map_LUT[it->second];
                }
                
                for( int h = 0; h < this->height; h++){
					for( int w = 0; w < this->width; w++){
                        if( *(this->at(w, h))){
                            int tempLabel = map_LUT[*(this->at(w, h))];
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
        
        template< typename T>
        class ImageCOG : public ImageAnalyzer<T, T>{
        public:
            double x, y;
        public:
            explicit ImageCOG(Image<T>& _srcImg)
            : ImageAnalyzer<T, T>( _srcImg.height, _srcImg.width, 0, _srcImg),
            x(_srcImg.width/2), y(_srcImg.height/2) {};
            virtual ~ImageCOG(void){};
        public:
            void execute(void){
                
                int cnt = 0;
                double num;
                double meanX, meanY;
                this->x = this->width / 2;
                this->y = this->width / 2;
                meanX = meanY = num = 0.0;
                for( int h = 0; h < this->height; h++){
                    for( int w = 0; w < this->width; w++){
                        if( *(this->im_roi.at(w, h)) ){
                            cnt++;
                            double value = static_cast<double>(*(this->srcImg.at(w, h)));
                            num   += value;
                            meanX += static_cast<double>(w) * value;
                            meanY += static_cast<double>(h) * value;
                        }
                    }
                }
                if( cnt && num > 0.0){
                    x = meanX / num;
                    y = meanY / num;
                }else{
                    x = -1;
                    y = -1;
                }
                *dynamic_cast<Image<T>*>(this) = this->srcImg;
            };
            
        };
        

        
        template< class SHRINKER, class ANALYZER, typename T_IN, typename T_OUT >
        class PyramidDetector : public ImageAnalyzer< T_IN, unsigned char >{
        public:
            int pyrNum;
            std::vector< SHRINKER* > vec_shrinker;
            std::vector< ANALYZER* > vec_analyzer;
            std::vector< MLARR::Analyzer::ImageDoubler<T_OUT>* > vec_doubler;
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
                    vec_doubler.push_back( new MLARR::Analyzer::ImageDoubler<T_OUT>( *vec_analyzer.back() ) );

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
                    /*
                    IO::Display<unsigned char> disp_roi_org("roi org",vec_analyzer[ i-1 ]->getRoi(),1,0,IO::colMap_gray);
                    IO::Display<unsigned char> disp_roi_dst("roi dst",imgHalf,1,0,IO::colMap_gray);
                    disp_roi_org.show();
                    disp_roi_dst.show();
                    cvWaitKey(-1);
                    */
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
                    vec_doubler[i-1]->execute();
                    //MorphImage<unsigned char> morph(vec_analyzer[i]->getBin(), 1);
                    //morph.execute();
                    //vec_analyzer[i-1]->setRoi( morph );
                    vec_analyzer[i-1]->setRoi( vec_doubler[i-1]->getBin() );
                }
                vec_analyzer[0]->execute();
                
                *dynamic_cast<Image<unsigned char>*>(this) = *dynamic_cast<Image<unsigned char>*>(vec_analyzer[0]);
                
            };
            
            void mergeSum(void){
                
                *(dynamic_cast<Image<unsigned char>*>(this)) = *vec_analyzer[0];
                for( int i = 1; i < vec_analyzer.size(); i++){
                    int size = pow(2,i);
                    ANALYZER* ptr = vec_analyzer[i];
                    for( int h = 0; h < ptr->height; h++){
                        for( int w = 0; w < ptr->width; w++){
                            for( int n = 0; n < size; n++){
                                for( int m = 0; m < size; m++){
                                    *this->at(w*size+m,h*size+n) += *ptr->at(w,h);
                                }
                            }
                        }
                    }
                }
                
            };
            
            void mergeFinest(void){
                std::vector< Image<unsigned char> > vecBuf;
                vecBuf.push_back( *vec_analyzer.back() );
                for( int i = static_cast<int>(vec_analyzer.size()) -  2; i >= 0; i--){
                    ImageDoubler<unsigned char> imgDouble(vecBuf.back());
                    BinaryAnd<unsigned char> imgAnd(imgDouble, *vec_analyzer[i]);
                    imgDouble.execute();
                    imgAnd.execute();
                    bool flg = false;
                    for( int h = 0; h < imgAnd.height; h++){
                        for( int w = 0; w < imgAnd.width; w++){
                            if(*imgAnd.at(w,h)){
                                flg = true;
                            }
                        }
                    }
                    if( flg ){
                        vecBuf.push_back(imgAnd);
                    }else{
                        vecBuf.push_back(imgDouble);
                    }
                }
                *dynamic_cast< Image<unsigned char>*>(this) = vecBuf.back();
            };
        };
        
	}

}