#pragma once

#include "stdafx.h"

namespace MLARR{

	namespace Basic{

		template<typename T>
		class Point
		{
		protected:
			T x;
			T y;
		public:
			Point(T _x, T _y ) : x(_x), y(_y){};
			Point(const Point& rhs ) : x( rhs.x ), y( rhs.y ){};
			virtual ~Point(void){};

			const T& getX(void){ return x; };
			const T& getY(void){ return y; };
			void setX(const T& _x){ x = _x; return; };
			void setY(const T& _y){ y = _y; return; };

			Point& operator=( const Point& rhs ){ x = rhs.x; y = rhs.y; return *this; };
			Point operator+( const Point& rhs ){ Point<T> ret( this->x+rhs.x, this->y+rhs.y); return ret;};
			Point operator-( const Point& rhs ){ Point<T> ret( this->x-rhs.x, this->y-rhs.y); return ret;};
			double operator*( const Point& rhs ){ return this->x*rhs.x + this->y*rhs.y; };
			double abs(void){ return sqrt( x * x + y * y ); };
		};

		
		/*------------------------------
		template<class T> class Vector
		{
		protected:
			T* data;
		public:
			const int length;
		public:
			Vector(const int _length, const T& iniValue) : length(_length) { 
				this->data = new T[this->length];
				new( this->data ) T(iniValue);
			};
			virtual ~Vector(void){ 
				delete this->vec;
			}
			void clear(const T& value){
				for(int h = 0; h < this->height; h++){
					this->data[h] = value;
				}
			};
			void clear(void){ 
				this->clear(static_cast<T>(0)); 
			};
			const T* const getRef(const int pos){ 
				return &(this->data[pos]); 
			}
			const T* const getRef(void){ 
				return this->getRef(0); 
			};
			T* getPtr(const int pos){ 
				return &(this->data[pos]); 
			}
			T* getPtr(void){ 
				return this->getPtr(0); 
			};
			void setValue( const int pos, const T& value){ 
				this->data[pos] = value; 
			}
		};*/

		/*------------------------------*/
		template<class T> class Image
		{
		protected:
			T* data;
		public:
			const int height;
			const int width;
		public:
			Image(const int _height, const int _width, const T& iniValue) : width(_width), height(_height) { 
				this->data = new T[this->nPix()];
				this->clear(iniValue);
			};
			Image(const Image<T>& rhs ) : width(rhs.width), height(rhs.height) {
				this->data = new T[this->nPix()];
				*this = rhs;
			};
			Image( const cv::Mat& src ) : width(), height(){
				this->data = new T[this->nPix()];
				*this = src;
			};
			Image( const std::vector<T>& src, int _width, int _height ) : width(_width), height(_height){
				this->data = new T[this->nPix()];
				if( this->nPix() == src.size()) *this = src;
			};
			virtual ~Image(void){ 
				delete this->data; 
			};
			Image<T>& operator=( const Image<T>& rhs ){
				if( width == rhs.width && height == rhs.height ){
					memcpy( data, rhs.data, sizeof(T)*this->nPix() );
				}
				return *this;
			};
			Image<T>& operator=( const cv::Mat& src ){
				T* ptr = this->getPtr();
				for(int h = 0; h < this->height; h++){
					for(int w = 0; w < this->width; w++){
						*ptr = src.at<T>(w, h);
						ptr++;
					}
				}
				return *this;
			};
			Image<T> operator+( const Image<T>& rhs ){
                Image<T> temp(*this);
                for( int n = 0; n < this->nPix(); n++){
                    temp.data[n] = this->data[n] + rhs.data[n];
                }
                return temp;
			};
			Image<T>& operator=( const std::vector<T>& src ){
				T* ptr = this->getPtr();
				int cnt = 0;
				for(int h = 0; h < this->height; h++){
					for(int w = 0; w < this->width; w++){
						*ptr = src[cnt];
						ptr++; cnt++;
					}
				}
				return *this;
			}
			void clear(const T& value){
				for(int h = 0; h < this->height; h++){
					for(int w = 0; w < this->width; w++){
						this->data[getPos(w, h)] = value;
					}
				}
			};
			void clear(void){ 
				this->clear(static_cast<T>(0)); 
			};
			int nPix(){ 
				return this->width*this->height; 
			};
			int getPos(const int w, const int h ) const{ 
				return  h * width + w ;
			};
			const T* const getRef(const int w, const int h) const{ 
				return &(this->data[ getPos(w,h) ]);
			};
			const T* const getRef(void) const{ 
				return this->getRef(0,0); 
			};
			T* getPtr(const int w, const int h) const{
				return &(this->data[ getPos(w,h) ]);
			};
			T* getPtr(void) const{
				return this->getPtr(0,0); 
			};
			void setValue( const int w, const int h, const T& value){ 
				this->data[ getPos(w,h) ] = value; 
			};
			cv::Mat* clone(void) const{
				int type = CV_8U;
				if( typeid(T) ==  typeid(unsigned char))   type = CV_8U;
				if( typeid(T) ==  typeid(char))            type = CV_8S;
				if( typeid(T) ==  typeid(unsigned short))  type = CV_16U;
				if( typeid(T) ==  typeid(short))           type = CV_16S;
				if( typeid(T) ==  typeid(float))           type = CV_32F;
				if( typeid(T) ==  typeid(double))          type = CV_64F;
				cv::Mat* retImg = new cv::Mat(this->height, this->width, type);
				T* ptr = this->getPtr();
				for(int h = 0; h < this->height; h++){
					for(int w = 0; w < this->width; w++){
						retImg->at<T>(w, h) = *ptr;
						ptr++;
					}
				}
				return retImg;
			};
		};


	}
}
