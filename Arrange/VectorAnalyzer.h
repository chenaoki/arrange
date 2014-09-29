#pragma once

#include "stdafx.h"
#include "coefs.h"
#include "io.h"
#include "analyzer.h"

namespace MLARR{

	namespace Analyzer{
        
        template <typename T>
        class NotchFilter : public TimeSeriesFilter<T,unsigned char>{
        protected:
            T upThre;
            T downThre;
        public:
            NotchFilter(void)
            : TimeSeriesFilter<T, unsigned char>(0), upThre(0.2), downThre(0.8)
            {};
            NotchFilter(T _upThre, T _downThre, unsigned char init)
            : TimeSeriesFilter<T, unsigned char>(init), upThre(_upThre), downThre(_downThre)
            {};
            NotchFilter(const NotchFilter<T>& rhs)
            : TimeSeriesFilter<T, unsigned char>(rhs.ret), upThre(rhs._upThre), downThre(rhs._downThre)
            {};
            ~NotchFilter(void){};
            NotchFilter<T>& operator=( const NotchFilter<T>& rhs ){
                this->ret = rhs.ret;
                this->upThre = rhs.upThre;
                this->downThre = rhs.downThre;
                this->buffer.clear();
                return *this;
            };
            void update(T& temp){
                this->buffer.push_back(temp);
                
                while( this->buffer.size() > 2) {
                    this->buffer.erase(this->buffer.begin());
                }
                
                if(this->buffer.size() == 2){
                    if( this->ret == 0 ){
                        if( this->buffer[0] < upThre && this->buffer[1] >= upThre){
                            this->ret = 1;
                        }
                    }else{
                        if( this->buffer[0] > downThre && this->buffer[1] <= downThre){
                            this->ret = 0;
                        }
                    }
                }
            };
            
        };
        
        template <typename T>
        class PositiveFilter : public TimeSeriesFilter<T, unsigned char>
        {
        public:
            PositiveFilter(void)
            : TimeSeriesFilter<T, unsigned char>(0)
            {};
            PositiveFilter(unsigned char init)
            : TimeSeriesFilter<T, unsigned char>(init)
            {};
            PositiveFilter(const NotchFilter<T>& rhs)
            : TimeSeriesFilter<T, unsigned char>(rhs.ret)
            {};
            ~PositiveFilter(void){};
            PositiveFilter<T>& operator=( const PositiveFilter<T>& rhs ){
                this->ret = rhs.ret;
                this->buffer.clear();
                return *this;
            };
            void update(T& temp){
                this->ret = temp > 0 ? 1 : 0;
            };
        };
        
		template<class T>
		class VectorAnalyzeFuncs{

		public:

			static void movingAveragefilter( const std::vector<T>& src, const int filterSize, std::vector<T>& dst ){

				if( filterSize > 0 ){

					/*copy src to dst*/
					dst.clear();
					for( typename std::vector<T>::const_iterator it = src.begin(); it != src.end(); it++ ){
						dst.push_back( static_cast<double>( *it ));
					}
					
					/*filtering*/
					typename std::vector<T>::const_iterator itvec = src.begin();
					for( int i = 0; i < src.size() - filterSize; i++ ){
						double val = 0.0;
						typename std::vector<T>::const_iterator it = itvec;
						for( int j = 0; j < filterSize; j++){
							val += static_cast<double>( *it );
							it++;
						}
						val /= filterSize;
						dst[i+filterSize/2] = static_cast<T>(val);
						itvec++;
					}

				}else{
					throw "filter size invalid.";
				}
			};


			static void stabilizeBaseLine( const std::vector<T>& src, const int minPeakDistance, const int filterSize, std::vector<T>& dst){

				std::vector<T> filSrcOnce;	
				std::vector<T> filSrc;	
				std::vector<T> revSrc(src.size());
				std::vector<T> maxLine(src.size());
				std::vector<T> minLine(src.size());
				std::vector<T> meanLine(src.size());
				std::map<int, T> maxMap;
				std::map<int, T> minMap;

				dst.clear();

				/* Moving Average */
				movingAveragefilter( src, filterSize, filSrcOnce );
				movingAveragefilter( filSrcOnce, filterSize, filSrc );
				for( int i = 0; i < filSrc.size(); i++) revSrc[i] = filSrc[i] * -1;
				findPeaks( filSrc, minPeakDistance, maxMap );
				findPeaks( revSrc, minPeakDistance, minMap );
				interpolation( maxMap, maxLine);
				interpolation( minMap, minLine);
				typename std::vector<T>::iterator it_max = maxLine.begin();
				typename std::vector<T>::iterator it_min = minLine.begin();
				typename std::vector<T>::iterator it_mean = meanLine.begin();
				for(; it_mean != meanLine.end(); it_mean++){
					*it_mean = (*it_max - *it_min ) / 2;
					it_max++;
					it_min++;
				}
				typename std::vector<T>::const_iterator it_fil;
				typename std::vector<T>::iterator it_dst;
				it_fil = filSrc.begin();
				it_mean = meanLine.begin();
				for(; it_mean!= meanLine.end(); it_mean++){
					dst.push_back( static_cast<T>(*it_fil) - *it_mean );
					it_fil++;
				}

				//
				//imgVec_src = src;
				//imgVec_fil = filSrc;
				//imgVec_rev = revSrc;
				//imgVec_max = maxLine;
				//imgVec_min = minLine;
				//imgVec_mea = meanLine;
				//imgVec_dst = dst;
				//plot_src.execute();
				//plot_fil.execute();
				//plot_rev.execute();
				//plot_max.execute();
				//plot_min.execute();
				//plot_mea.execute();
				//plot_dst.execute();
				//disp_plot_src.show();
				//disp_plot_fil.show();
				//disp_plot_rev.show();
				//disp_plot_max.show();
				//disp_plot_min.show();
				//disp_plot_mea.show();
				//disp_plot_dst.show();


			};

			static void findPeaks(const std::vector<T>& srcVec, const int minPeakDistance, std::map<int, T>& peaks ){
				peaks.clear();
				for( int i = 1; i < srcVec.size()-1; i++){
					if( srcVec[i-1] < srcVec[i] && srcVec[i] > srcVec[i+1]){
						peaks.insert( std::pair<int, T>( i, srcVec[i] ));
						i += minPeakDistance - 1;
					}
				}
			};

			static void interpolation(const std::map<int, T>& peaks, std::vector<T>& dstVec ){
				if( peaks.size() >= 2 ){
					typename std::map<int, T>::const_iterator itmap_pre = peaks.begin();
					typename std::map<int, T>::const_iterator itmap_pst = peaks.begin();
					itmap_pst++;
					for( int i = 0; i < dstVec.size(); i++){

						if( itmap_pst != peaks.end()){
							if( i >= static_cast<int>(itmap_pst->first) ){
								itmap_pst++;
								itmap_pre++;
							}
						}

						if( itmap_pst == peaks.end() || i <= static_cast<int>(itmap_pre->first) ){
							dstVec[i] = itmap_pre->second;
						}else{
							T preVal = itmap_pre->second;
							T pstVal = itmap_pst->second;
							T amplitude = pstVal - preVal;
							float width = static_cast<float>(itmap_pst->first - itmap_pre->first);
							if( width != 0 ){
								dstVec[i] = ( preVal + amplitude * ( i - itmap_pre->first ) / width);
							}else{
								throw "error";
							}
						}
					}
				}
			};

		};

	}
}

