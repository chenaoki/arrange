//------------------------------------------------------------------------------
// ヒルベルト変換用フィルタの係数
//      ヒルベルト変換用フィルタの仕様
//          次数         230
//          通過域の低域端周波数  0.2 kHz
//          通過域の高域端周波数 23.8 kHz
//          通過域の偏差   　     0.17596957 dB
//------------------------------------------------------------------------------
#pragma once 

#include "stdafx.h"

namespace MLARR{

	namespace Analyzer{

		class Coeffs{
		public:
			/*
			static const int HILBERT_ORDER = 230;
			*/
			static const int HILBERT_ORDER = 20;
			static const double HILBERT_COEFS[HILBERT_ORDER+1];
			
			static const double GAUSSIAN_3x3[9];
			static const double GAUSSIAN_5x5[25];
			static const int SPFIR_DIM = 6;
			static const double SPFIR_COEFS[SPFIR_DIM];
            static const double NABLA_X_COEFS[9];
            static const double NABLA_Y_COEFS[9];
            static const unsigned char RGB_HSV[256][3];
            
			std::vector<double> vec_gaussian_3x3;
			std::vector<double> vec_gaussian_5x5;
			std::vector<double> vec_spFIR;
            std::vector<double> vec_nablaX;
            std::vector<double> vec_nablaY;
		public:
			Coeffs(void) : vec_gaussian_5x5(){
				for(int i = 0; i < 25; i++){
					vec_gaussian_5x5.push_back( GAUSSIAN_5x5[i] );
				}
				for(int i = 0; i < SPFIR_DIM; i++){
					vec_spFIR.push_back(SPFIR_COEFS[i]);
				}
                for(int i = 0; i < 9; i++){
                    vec_gaussian_3x3.push_back(GAUSSIAN_3x3[i]);
                }
                for( int i = 0; i < 9; i++ ){
                    vec_nablaX.push_back(NABLA_X_COEFS[i]);
                    vec_nablaY.push_back(NABLA_Y_COEFS[i]);
                }
			}
		};

		extern Coeffs coefficients;
	}

}
