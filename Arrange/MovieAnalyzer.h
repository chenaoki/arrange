#include "stdafx.h"

#include "basic.h"
#include "io.h"
#include "analyzer.h"
#include "vectoranalyzer.h"

namespace MLARR{
	namespace Analyzer{

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
		
		template<class T>
		class HilbertAnalyzer : public MovieAnalyzer<T, double>
		{
		public:
			const int start;
			const int skip;
			const int stop;
			const int minPeakDistance;
			const int filterSize;
		public:
			HilbertAnalyzer( Image<T>& _srcImg, int _minPeakDistance, int _filterSize, int _start, int _skip, int _stop )
				: MovieAnalyzer<T, double>( _srcImg, _stop, _stop, _start ), filterSize(_filterSize) ,minPeakDistance(_minPeakDistance), start(_start), skip(_skip), stop(_stop){
			};
			~HilbertAnalyzer(void){};
			void execute(void){
				if( !this->flagValid ){

					/* Initialize output buffer */
					this->outImgBuf.clear();
					while( this->outImgBuf.size() < this->srcImgBuf.size()){
						this->outImgBuf.push_back( Image<double>( this->srcImg.height, this->srcImg.width, 0.0 ) );
					}

					MLARR::Basic::Image<unsigned char> img_prog( this->width, this->height, 0);
					MLARR::IO::Display<unsigned char> disp_prog( "hilbert progress", img_prog, 1, 0, MLARR::IO::colMap_hsv );

					std::vector<T> srcVec;
					std::vector<T> srcStab;
					for( int h = 0; h < this->height; h++){
						for( int w = 0; w < this->width; w++){		
							if( *(this->im_roi.getRef(w, h)) ){
								/* Stabilize signal */
								srcVec.clear();
								srcStab.clear();
								typename std::vector<Image<T>>::iterator it_img = this->srcImgBuf.begin();
								for( it_img = this->srcImgBuf.begin(); it_img != this->srcImgBuf.end(); it_img++){
									srcVec.push_back( *(it_img->getRef(w, h)));
								}
								MLARR::Analyzer::VectorAnalyzeFuncs<T>::stabilizeBaseLine( 
									srcVec, 
									minPeakDistance, 
									filterSize,
									srcStab );
								
								/* Hilbert Filtering */
								HilbertFilter<double, MLARR::Analyzer::Coeffs::HILBERT_ORDER> hilbert(MLARR::Analyzer::Coeffs::HILBERT_COEFS);
								std::vector<double>::iterator it_vec = srcStab.begin();
								std::vector<Image<double>>::iterator it_imgOut = this->outImgBuf.begin();
								for( ; it_vec != srcStab.end(); it_vec++){
									double y_real, y_im;
									hilbert.execute( *it_vec, y_real, y_im );

									/* Calc phase value */
									double phase = atan2( y_im , y_real );
									it_imgOut->setValue( w, h, phase );
									it_imgOut++;
								}
							}
							img_prog.setValue(w, h, 1);
						}
						disp_prog.show();
					}
					this->flagValid = 1;
				}
				if( this->nOutPos >= 0 && this->nOutPos < this->outImgBuf.size()){
					*(dynamic_cast<MLARR::Basic::Image<double>*>(this)) = this->outImgBuf[this->nOutPos];
				}
				if( ( this->nOutPos += skip ) > stop-1 ) this->nOutPos = stop-1;
			};


		};


	}
}