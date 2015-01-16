#pragma once

#ifndef __MLARR_LIB_IO_H__
#define __MLARR_LIB_IO_H__

#include "stdafx.h"
#include "Basic.h"
#include "coefs.h"
#include "IO.h"

using namespace MLARR::Basic;

namespace MLARR{

	namespace IO{

		enum ECamState{ standby, run, stop, error };

		enum EColor{ white = 0, black, red, green, blue };
        
		const cv::Scalar arrColor[5] = {
			cv::Scalar(255,255,255),
			cv::Scalar(0,0,0),
			cv::Scalar(0,0,200),
			cv::Scalar(0,200,0),
			cv::Scalar(200,0,0)
		};
        
        extern void brendColor( int numColor1, int numColor2, double alpha, cv::Vec3b &pixVal);

        picojson::object loadJsonParam(const std::string& paramFilePath);

		template<class T> class ICamera : public Image<T>
		{
		public:
			size_t bits;
			double fps;
			int state;
			int f_tmp;
			int f_next;
			int msec;
		public:
			ICamera<T>( int imgWidth, int imgHeight, size_t _bits, double _fps ) 
				: Image<T>( imgWidth, imgHeight, static_cast<T>(0) ), bits(_bits), fps(_fps), state( standby ), f_tmp(0), f_next(1), msec(0){
			};
			virtual ~ICamera(void){};
			const int& getTime( void ){
				msec = static_cast<int>( this->f_tmp * 1000 / static_cast<double>(this->fps));
				return msec;
			};
			virtual void capture(void) = 0;
			virtual void initialize(void){
				this->f_tmp = 0;
				this->f_next = 1;
				this->state = standby;
			};
			
		};

		template<class T>
		class RawFileCamera : public ICamera<T>
		{
		public:
			const int f_start;
			const int f_skip;
			const int f_stop;
			const std::string dirPath;
			const std::string format;
		public:
			RawFileCamera( int imgWidth, int imgHeight, size_t _bits, double _fps, const std::string& _dirPath, const std::string& _format, const int _f_start, const int _f_skip, const int _f_stop ) 
				: ICamera<T>( imgWidth, imgHeight, _bits, _fps), dirPath(_dirPath), format(_format), f_start(_f_start), f_skip(_f_skip), f_stop(_f_stop){
                    
					this->f_tmp = f_start;
			};
			virtual ~RawFileCamera(void){};
			void capture(void){
				if( this->state == standby || this->state == run ){
					char path[255];
					sprintf( path, format.c_str(), dirPath.c_str(), this->f_next);
					std::ifstream fin(path, std::ios::in | std::ios::binary );
					if( fin ){
						for( int h = 0; h < this->height; h++){
							for(int w = 0; w < this->width; w++){
								if( !fin.eof()){
									T src, dst; 
									fin.read( (char*)(&src), sizeof(T) );
									this->binaryTrans( src, dst );
									this->setValue(w, h, dst);
								}
							}
						}
						this->f_tmp = this->f_next;
						this->f_next+=this->f_skip;
						this->state = run;
					}else{
						this->state = stop;
					}
					fin.close();

					if( run == this->state ){
						if( this->f_stop <= this->f_tmp ){
							this->state = stop;
						}
					}
				}
			};
			void initialize(void){
				ICamera<T>::initialize();
				this->f_next = f_start;
			}
		protected:
			virtual void binaryTrans( const T& src, T& dst){dst = src;};
		};

		class DalsaRawFileCamera : public RawFileCamera<unsigned short>{
		public:
			DalsaRawFileCamera( const std::string& _dirPath, const std::string& _format, const int _f_start, const int _f_skip, const int _f_stop, int _size = 128, int _fps = 500 )
				: RawFileCamera<unsigned short>( _size, _size, 12, _fps, _dirPath, _format, _f_start, _f_skip, _f_stop ){
			};
			~DalsaRawFileCamera(void){};
		protected:
			void binaryTrans( const unsigned short& src, unsigned short& dst){
				dst = ( src & 0xF ) << 8;
				dst+= src >> 8;
			};
		};
        
        class HurricaneRawFileCamera : public RawFileCamera<unsigned short>{
        public:
            HurricaneRawFileCamera( const std::string& _dirPath, const std::string& _format, const int _f_start, const int _f_skip, const int _f_stop, int _size = 512, int _fps = 1000 )
            : RawFileCamera<unsigned short>( _size, _size, 8, _fps, _dirPath, _format, _f_start, _f_skip, _f_stop ){
            };
            ~HurricaneRawFileCamera(void){};
        protected:
            void binaryTrans( const unsigned short& src, unsigned short& dst){
                /* do nothing */
                dst = src;
            };
        };

		class MaxRawFileCamera : public RawFileCamera<unsigned short>{
		public:
			MaxRawFileCamera( const std::string& _dirPath, const std::string& _format, const int _f_start, const int _f_skip, const int _f_stop, int _size = 512, int _fps = 1000 ) 
				: RawFileCamera<unsigned short>( _size, _size, 10, _fps, _dirPath, _format, _f_start, _f_skip, _f_stop ){
			};
			~MaxRawFileCamera(void){};
		protected:
			void binaryTrans( const unsigned short& src, unsigned short& dst){
				/* do nothing */
				dst = src;
			};
		};
        
        class SA4RawFileCamera : public RawFileCamera<unsigned short>{
        public:
            SA4RawFileCamera( const std::string& _dirPath, const std::string& _format, const int _f_start, const int _f_skip, const int _f_stop, int _size = 512, int _fps = 1000 )
            : RawFileCamera<unsigned short>( _size, _size, 12, _fps, _dirPath, _format, _f_start, _f_skip, _f_stop )
            {};
            ~SA4RawFileCamera(void){};
        protected:
			void binaryTrans( const unsigned short& src, unsigned short& dst){
				/* do nothing */
				dst = src;
			};
        };
        
        class Mono8RawFileCamera : public RawFileCamera<unsigned char>{
        public:
            Mono8RawFileCamera( const std::string& _dirPath, const std::string& _format, const int _f_start, const int _f_skip, const int _f_stop, int _size, int _fps ) : RawFileCamera<unsigned char>( _size, _size, 8, _fps, _dirPath, _format, _f_start, _f_skip, _f_stop )
            {};
            ~Mono8RawFileCamera(void){};
        protected:
            void binaryTrans( const unsigned char& src, unsigned char& dst){
				/* do nothing */
				dst = src;
			};
        };
        
		class ColorMap
		{
		protected:
			unsigned char red[UCHAR_MAX+1];
			unsigned char green[UCHAR_MAX+1];
			unsigned char blue[UCHAR_MAX+1];
		public:
			ColorMap(void){};
			virtual~ColorMap(void){};
			void getColor(const unsigned char& value, char& red, char& green, char& blue) const{
				red = this->red[value];
				green = this->green[value];
				blue = this->blue[value];
			};
		};

		class GrayColorMap : public ColorMap
		{
		public:
			GrayColorMap(void){
				for(int i = 0; i <= UCHAR_MAX; i++){
					red[i] = green[i] = blue[i] = i;
				}
			};
			~GrayColorMap(void){};
		};
		class OrangeColorMap : public ColorMap
		{
		public:
			OrangeColorMap(void){
				for( unsigned int c =0; c <= UCHAR_MAX; c++){
					if ( c == 0 ) {
						blue[c] =0;
						green[c] =0;
						red[c] =0;
					}else if ( c < 24 ) {
						blue[c] =228-((600/22)*(c - 1))/100;
						green[c] =147-((4000/22)*(c - 1))/100;
						red[c] =65-((6500/22)*(c - 1))/100;
					}else if ( c < 75 ) {
						blue[c] =228-((11500/50)*(c - 24))/100;
						green[c] =107-((10700/50)*(c - 24))/100;
						red[c] =0;
					}else if ( c < 107 ) {
						blue[c] =113-((6500/31)*(c - 75))/100;
						green[c] =0;
						red[c] =0;
					}else if ( c < 130 ) {
						blue[c] =48-((4800/22)*(c - 107))/100;
						green[c] =0;
						red[c] =((7200/22)*(c - 107))/100;
					}else if ( c < 169 ) {
						blue[c] =0;
						green[c] =0;
						red[c] =72+((18300/38)*(c - 130))/100;
					}else if ( c < 212 ) {
						blue[c] =0;
						green[c] =((20600/42)*(c - 169))/100;
						red[c] =255;
					}else if ( c < 231 ) {
						blue[c] =((3700/18)*(c - 212))/100;
						green[c] =206+((4900/18)*(c - 212))/100;
						red[c] =255;
					}else if ( c <= UCHAR_MAX ) {
						blue[c] =37+((21800/24)*(c - 231))/100;
						green[c] =255;
						red[c] =255;
					}
				}
			};
			~OrangeColorMap(void){};
		};
        extern OrangeColorMap colMap_orange;

		class HSVColorMap : public ColorMap
		{
		public:
			HSVColorMap(void){
				for( int i = 0; i < 256; i++){
					red[i]   = MLARR::Analyzer::coefficients.RGB_HSV[i][0];
					green[i] = MLARR::Analyzer::coefficients.RGB_HSV[i][1];
					blue[i]  = MLARR::Analyzer::coefficients.RGB_HSV[i][2];
				}
			};
			~HSVColorMap(void){};
		};
        
        extern HSVColorMap colMap_hsv;
        extern OrangeColorMap colMap_orange;
        extern GrayColorMap colMap_gray;
        

		template<class T> class Dumper
		{
		private:
			const Image<T>& srcImage;
			const std::string saveDir;
			const std::string format;
		public:
			Dumper( const Image<T>& _srcImage, const std::string _saveDir, const std::string& _format)
				: srcImage(_srcImage), saveDir(_saveDir), format(_format){
			};
			Dumper( const std::vector<T>& _srcVector, const std::string _saveDir, const std::string& _format)
				: srcImage(_srcVector), saveDir(_saveDir), format(_format){
			};
			~Dumper(void){};
			void dump(const int fnum){
				char path[255];
				sprintf( path, format.c_str(), saveDir.c_str(), fnum);
				std::ofstream fout(path, std::ios::out | std::ios::binary | std::ios::trunc );
				if( fout ){
					for(int h = 0; h < srcImage.height; h++){
						for( int w=0; w < srcImage.width; w++){
							fout.write( (char*)(this->srcImage.at(w,h)), sizeof(T));
						}
					}
				}
				fout.close();
			};
            void dumpText(const int fnum){
				char path[255];
                std::string fmt = format + ".txt";
				sprintf( path, fmt.c_str(), saveDir.c_str(), fnum);
				std::ofstream fout(path, std::ios::out | std::ios::trunc );
				if( fout ){
					for(int h = 0; h < srcImage.height; h++){
						for( int w=0; w < srcImage.width; w++){
							fout << *this->srcImage.at(w,h) << ",";
						}
                        fout << std::endl;
					}
				}
				fout.close();
            }

		};


		template<class T> class Display
		{
		protected:
			Image<T>& srcImg;
			cv::Mat cv_img;
			const ColorMap& colMap;
			const std::string winName;
			const T maxVal;
			const T minVal;
			const int strFontStyle;
			const double strFontSize;
			const cv::Point strDrawPoint;
            int flg_update;
		protected:
			void updateCVImage(void){
				cv::Mat imgTemp(srcImg.height, srcImg.width, CV_8UC3);
				for( int h = 0; h < this->srcImg.height; h++){
					for(int w = 0; w < this->srcImg.width; w++){
						char r, g, b;
						cv::Vec3b pixVal;
						T val = *(this->srcImg.at( w, h ));
						unsigned char v = static_cast<unsigned char>( UCHAR_MAX * ( val - minVal ) / (maxVal - minVal) );
						this->colMap.getColor( v, r, g, b);
						pixVal[0] = b;
						pixVal[1] = g;
						pixVal[2] = r;
						imgTemp.at<cv::Vec3b>( h, w ) = pixVal;
					}
				}
				cv::resize(imgTemp, cv_img, cv_img.size(), 0,0, cv::INTER_NEAREST );
			};
		public:
			explicit Display<T>( const std::string _winName, Image<T>& _srcImage, const T& _maxVal, const T& _minVal, const ColorMap& _colMap ) 
				: winName(_winName), srcImg(_srcImage), maxVal(_maxVal), minVal(_minVal) ,colMap(_colMap), cv_img(_srcImage.height, _srcImage.width, CV_8UC3), strFontSize(0.4), strFontStyle(cv::FONT_HERSHEY_SIMPLEX), strDrawPoint(1,15), flg_update(0){
					cv::namedWindow(winName);
			};
			Display<T>( const std::string _winName, Image<T>& _srcImage, const T& _maxVal, const T& _minVal, const ColorMap& _colMap, const int dispWidth, const int dispHeight )
				: winName(_winName), srcImg(_srcImage), maxVal(_maxVal), minVal(_minVal) ,colMap(_colMap), cv_img(dispHeight, dispWidth, CV_8UC3), strFontSize(0.4), strFontStyle(cv::FONT_HERSHEY_SIMPLEX), strDrawPoint(1,15), flg_update(0){
					cv::namedWindow(winName);
			}
			virtual ~Display(void){
				cv::destroyWindow(winName);
			};
            void drawRect( int left, int top, int right, int bottom, int color ){
				if( !flg_update ){ updateCVImage(); flg_update = 1; }
                cv::rectangle(cv_img, cv::Point(left, top), cv::Point(right, bottom), arrColor[color]);
            };
            void drawRect( int x, int y, int rad, int color){
                if( !flg_update ){ updateCVImage(); flg_update = 1; }
                cv::rectangle(cv_img, cv::Point(x - rad, y - rad), cv::Point(x + rad, y + rad), arrColor[color]);
            };
            void drawMask( const Image<unsigned char>& _mskImage, int color ){
                if( !flg_update ){ updateCVImage(); flg_update = 1; }
                cv::Vec3b pixVal;
                pixVal[0] = arrColor[color].val[0];
                pixVal[1] = arrColor[color].val[1];
                pixVal[2] = arrColor[color].val[2];
                for( int h = 0; h < _mskImage.height; h++){
                    for( int w = 0; w < _mskImage.width; w++){
                        if( *_mskImage.at(w, h) ){
                            cv_img.at<cv::Vec3b>( h, w ) = pixVal;
                        }
                    }
                }
            };
            void drawMask( const Image<unsigned char>& _mskImage, cv::Vec3b &pixVal ){
                if( !flg_update ){ updateCVImage(); flg_update = 1; }
                for( int h = 0; h < _mskImage.height; h++){
                    for( int w = 0; w < _mskImage.width; w++){
                        if( *_mskImage.at(w, h) ){
                            cv_img.at<cv::Vec3b>( h, w ) = pixVal;
                        }
                    }
                }
            };
			void show(void){
				if( !flg_update ) updateCVImage();
				cv::imshow(winName, cv_img);
				cv::waitKey(10);
                flg_update = 0;
			};
			void show(const std::string& str, int color){
				if( !flg_update ) updateCVImage();
				cv::putText(cv_img, str.c_str(), strDrawPoint, strFontStyle, strFontSize, arrColor[color] );
				cv::imshow(winName, cv_img);
				cv::waitKey(10);
                flg_update = 0;
			};
			void show(const int msec, int color){
				if( !flg_update ) updateCVImage();
				char cstr[255];
				sprintf( cstr, "%dms", msec );
				cv::putText(cv_img, cstr, strDrawPoint, strFontStyle, strFontSize, arrColor[color] );
				cv::imshow(winName, cv_img);
				cv::waitKey(10);
                flg_update = 0;
			};
			void save( const std::string& saveDir, const std::string& format, const int frameNum ){
				char path[255];
				sprintf( path, format.c_str(), saveDir.c_str(), frameNum);
				cv::imwrite( path, cv_img);
			};
		};
        

		/*
        template< class T, class T_X > class PlotDisplay : public Display<unsigned char>
		{
		protected:
			MLARR::Analyzer::Plotter<T>& plot;
			T_X start_x;
			T_X step_x;
			const T_X scale_x;
			const T   scale_y;
		public:
			PlotDisplay( const std::string _winName, MLARR::Analyzer::Plotter<T>& _srcImage, const T_X _scale_x = 0, const T _scale_y = 0) 
				: Display<unsigned char>(_winName, dynamic_cast<MLARR::Basic::Image<unsigned char>&>(_srcImage), 1, 0, colMap_gray), plot(_srcImage), start_x(0), step_x(1), scale_x(_scale_x), scale_y( _scale_y ) {
					cv::namedWindow(winName);
			};
			PlotDisplay( const std::string _winName, MLARR::Analyzer::Plotter<T>& _srcImage, const int dispWidth, const int dispHeight, const T_X _scale_x = 0, const T _scale_y = 0 )
				: Display<unsigned char>(_winName, dynamic_cast<MLARR::Basic::Image<unsigned char>&>(_srcImage), 1, 0, colMap_gray, dispWidth, dispHeight), plot(_srcImage), start_x(0), step_x(1), scale_x(_scale_x), scale_y( _scale_y ){
					cv::namedWindow(winName);
			}
			virtual ~PlotDisplay(void){
				cv::destroyWindow(winName);
			};
			void setXaxis( const T_X _start_x, const T_X _step_x, const T_X _scale_x){
				start_x = _start_x; 
				step_x = _step_x; 
			};
			void setScale( const T_X _scale_x, const T _scale_y){
				scale_x = _scale_x;
				scale_y = _scale_y;
			};
			void show(void){
				updateCVImage();
				cv::imshow(winName, cv_img);
				cv::waitKey(10);
			};
		protected:
			void updateCVImage(void){

				cv::Mat imgTemp = cv::Mat::zeros(srcImg.height, srcImg.width, CV_8UC3);
				for(int w = 0; w < this->plot.width-1; w++){
					for( int h = 0; h < this->plot.height; h++){
						cv::Vec3d val(255,255,255);
						imgTemp.at<cv::Vec3b>( h, w ) = val;
					}
				}

				// draw plot
				int h_pre = -1;
				int h_pst = -1;				
				for(int w = 0; w < this->plot.width-1; w++){
					for( int h = 0; h < this->plot.height; h++){
						h_pre = *(this->srcImg.at( w, h )) == 0 ? h : h_pre;
						h_pst = *(this->srcImg.at( w+1, h )) == 0 ? h : h_pst;
					}
					if( h_pre > 0 && h_pst > 0 ){
						cv::line(imgTemp, cvPoint( w, h_pre ), cvPoint( w+1, h_pst), strColor[black] );
					}
				}

				// search zero line and draw
				int zero_w = -1;
				int zero_h = -1;
				size_t x;
				for(int w = 0; w < this->plot.width; w++){
					x = w * step_x + start_x;
					if( x == static_cast<T_X>(0)) zero_w = w;
				};
				size_t tmpH = plot.getHeight(0);
				if( tmpH >= 0 && tmpH < plot.height ) zero_h = tmpH;
				if( zero_w >= 0 ){
					cv::line(imgTemp, cvPoint( zero_w, 0 ), cvPoint( zero_w, plot.height - 1 ), strColor[black] );
				}else{
					zero_w = 0;
				}
				if( zero_h >= 0 ){
					cv::line(imgTemp, cvPoint( 0, zero_h ), cvPoint( plot.width - 1, zero_h ), strColor[black] );
				}else{
					zero_h = 0;
				}

				// draw scale
				char buf[20];
				if( scale_x > 0 ){
					for(int w = 0; w < this->srcImg.width; w++){
						x = w * step_x + start_x;
						if( x - static_cast<int>( x / scale_x) * scale_x < step_x ){
							sprintf(buf, "%.1f", static_cast<int>( x / scale_x) * scale_x );
							cv::putText(cv_img, buf, cvPoint(w, zero_h), strFontStyle, strFontSize, strColor[black] );
						}
					}
				}
				if( scale_y > 0 ){
					double step_y = plot.getHeightValue(1) - plot.getHeightValue(0);
					for(int h = 0; h < this->plot.height; h++){
						T y = plot.getHeightValue(h);
						if( y - static_cast<int>( y / scale_y) * scale_y < step_y ){
							sprintf(buf, "%.1f", static_cast<int>( y / scale_y) * scale_y );
							cv::putText(cv_img, buf, cvPoint(zero_w, h), strFontStyle, strFontSize, strColor[black] );
						}
					}
				}

				// reflect to display
				cv::resize(imgTemp, cv_img, cv_img.size(), 0,0, cv::INTER_NEAREST );

			};
		};
        */
        
	}
}

#endif // __MLARR_LIB_IO_H__
