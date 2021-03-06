#include "stdafx.h"
#include "coefs.h"

MLARR::Analyzer::Coeffs MLARR::Analyzer::coefficients;

const double MLARR::Analyzer::Coeffs::HILBERT_COEFS[HILBERT_ORDER+1] = {
/*		-1.10216831e-02, -5.35235376e-08, -1.62741486e-03,  9.19962458e-07,
		-1.74850348e-03, -2.00519718e-06, -1.87219781e-03,  2.34178138e-06,
		-2.00601700e-03, -2.73175426e-07, -2.14448814e-03, -9.34700849e-07,
		-2.28754302e-03, -8.25998933e-07, -2.43947457e-03,  5.38942588e-07,
		-2.59837203e-03,  9.06487236e-07, -2.76513999e-03, -4.18722090e-07,
		-2.93876327e-03, -1.25863103e-06, -3.12055483e-03, -1.44895154e-06,
		-3.31097292e-03, -6.56468906e-07, -3.51019582e-03, -3.19115408e-07,
		-3.71917864e-03, -3.75253200e-07, -3.93814423e-03, -6.02714618e-07,
		-4.16789478e-03, -8.79949234e-07, -4.40826484e-03, -9.97192787e-07,
		-4.66000529e-03, -1.21345814e-06, -4.92412882e-03, -1.10763134e-06,
		-5.20173440e-03, -8.86491063e-07, -5.49382760e-03, -5.37789933e-07,
		-5.80096419e-03, -2.51355452e-07, -6.12438134e-03, -1.12788254e-07,
		-6.46524157e-03, -7.91588657e-08, -6.82509018e-03, -3.71474113e-07,
		-7.20536228e-03, -7.69769481e-07, -7.60776848e-03, -1.18010619e-06,
		-8.03448196e-03, -1.31197493e-06, -8.48779891e-03, -1.23254109e-06,
		-8.97054257e-03, -1.10881249e-06, -9.48568815e-03, -9.72386838e-07,
		-1.00369103e-02, -9.04935072e-07, -1.06283178e-02, -8.00951807e-07,
		-1.12647159e-02, -8.02851976e-07, -1.19518475e-02, -8.52936880e-07,
		-1.26964626e-02, -9.31376277e-07, -1.35068058e-02, -9.71168104e-07,
		-1.43925564e-02, -9.18102067e-07, -1.53656559e-02, -8.53576731e-07,
		-1.64407606e-02, -7.08713995e-07, -1.76362189e-02, -5.65139639e-07,
		-1.89751321e-02, -3.80211937e-07, -2.04868223e-02, -2.04362720e-07,
		-2.22094356e-02, -2.73497323e-08, -2.41931993e-02,  1.76555662e-07,
		-2.65059794e-02,  3.17267283e-07, -2.92413743e-02,  3.98600530e-07,
		-3.25324754e-02,  3.29549943e-07, -3.65750919e-02,  1.94514463e-07,
		-4.16694574e-02,  6.34054189e-09, -4.83007342e-02, -1.90607627e-07,
		-5.73067941e-02, -3.48293361e-07, -7.02705135e-02, -4.69268207e-07,
		-9.05836979e-02, -4.60905853e-07, -1.27065249e-01, -3.79771652e-07,
		-2.12051331e-01, -2.00431727e-07, -6.36568003e-01,  0.00000000e+00,
		 6.36568003e-01,  2.00431727e-07,  2.12051331e-01,  3.79771652e-07,
		 1.27065249e-01,  4.60905853e-07,  9.05836979e-02,  4.69268207e-07,
		 7.02705135e-02,  3.48293361e-07,  5.73067941e-02,  1.90607627e-07,
		 4.83007342e-02, -6.34054189e-09,  4.16694574e-02, -1.94514463e-07,
		 3.65750919e-02, -3.29549943e-07,  3.25324754e-02, -3.98600530e-07,
		 2.92413743e-02, -3.17267283e-07,  2.65059794e-02, -1.76555662e-07,
		 2.41931993e-02,  2.73497323e-08,  2.22094356e-02,  2.04362720e-07,
		 2.04868223e-02,  3.80211937e-07,  1.89751321e-02,  5.65139639e-07,
		 1.76362189e-02,  7.08713995e-07,  1.64407606e-02,  8.53576731e-07,
		 1.53656559e-02,  9.18102067e-07,  1.43925564e-02,  9.71168104e-07,
		 1.35068058e-02,  9.31376277e-07,  1.26964626e-02,  8.52936880e-07,
		 1.19518475e-02,  8.02851976e-07,  1.12647159e-02,  8.00951807e-07,
		 1.06283178e-02,  9.04935072e-07,  1.00369103e-02,  9.72386838e-07,
		 9.48568815e-03,  1.10881249e-06,  8.97054257e-03,  1.23254109e-06,
		 8.48779891e-03,  1.31197493e-06,  8.03448196e-03,  1.18010619e-06,
		 7.60776848e-03,  7.69769481e-07,  7.20536228e-03,  3.71474113e-07,
		 6.82509018e-03,  7.91588657e-08,  6.46524157e-03,  1.12788254e-07,
		 6.12438134e-03,  2.51355452e-07,  5.80096419e-03,  5.37789933e-07,
		 5.49382760e-03,  8.86491063e-07,  5.20173440e-03,  1.10763134e-06,
		 4.92412882e-03,  1.21345814e-06,  4.66000529e-03,  9.97192787e-07,
		 4.40826484e-03,  8.79949234e-07,  4.16789478e-03,  6.02714618e-07,
		 3.93814423e-03,  3.75253200e-07,  3.71917864e-03,  3.19115408e-07,
		 3.51019582e-03,  6.56468906e-07,  3.31097292e-03,  1.44895154e-06,
		 3.12055483e-03,  1.25863103e-06,  2.93876327e-03,  4.18722090e-07,
		 2.76513999e-03, -9.06487236e-07,  2.59837203e-03, -5.38942588e-07,
		 2.43947457e-03,  8.25998933e-07,  2.28754302e-03,  9.34700849e-07,
		 2.14448814e-03,  2.73175426e-07,  2.00601700e-03, -2.34178138e-06,
		 1.87219781e-03,  2.00519718e-06,  1.74850348e-03, -9.19962458e-07,
		 1.62741486e-03,  5.35235376e-08,  1.10216831e-02*/
 
    0.0000,
   -0.0961,
   -0.0000,
   -0.0739,
   -0.0000,
   -0.1146,
   -0.0000,
   -0.2043,
    0.0000,
   -0.6339,
         0,
    0.6339,
   -0.0000,
    0.2043,
    0.0000,
    0.1146,
    0.0000,
    0.0739,
    0.0000,
    0.0961,
   -0.0000
};

const double MLARR::Analyzer::Coeffs::GAUSSIAN_5x5[25] = {
		1  / static_cast<double>(256),
		4  / static_cast<double>(256),
		6  / static_cast<double>(256),
		4  / static_cast<double>(256),
		1  / static_cast<double>(256),
		4  / static_cast<double>(256),
		16 / static_cast<double>(256),
		24 / static_cast<double>(256),
		16 / static_cast<double>(256),
		4  / static_cast<double>(256),
		6  / static_cast<double>(256),
		24 / static_cast<double>(256),
		36 / static_cast<double>(256),
		24 / static_cast<double>(256),
		6  / static_cast<double>(256),
		4  / static_cast<double>(256),
		16 / static_cast<double>(256),
		24 / static_cast<double>(256),
		16 / static_cast<double>(256),
		4  / static_cast<double>(256),
		1  / static_cast<double>(256),
		4  / static_cast<double>(256),
		6  / static_cast<double>(256),
		4  / static_cast<double>(256),
		1  / static_cast<double>(256)};

const double MLARR::Analyzer::Coeffs::GAUSSIAN_3x3[9] = {
    1  / static_cast<double>(16),
    2  / static_cast<double>(16),
    1  / static_cast<double>(16),
    2  / static_cast<double>(16),
    4  / static_cast<double>(16),
    2  / static_cast<double>(16),
    1  / static_cast<double>(16),
    2  / static_cast<double>(16),
    1  / static_cast<double>(16)};

const double MLARR::Analyzer::Coeffs::SPFIR_COEFS[SPFIR_DIM] = { 
		1.6747156, 0.6397844, 0.3998027, -0.3998027, -0.6397844, -1.6747156};


const double MLARR::Analyzer::Coeffs::NABLA_X_COEFS[9] = {
   -0.5,  0, 0.5,
     -1, 0,   1,
   -0.5,  0, 0.5
};

const double MLARR::Analyzer::Coeffs::NABLA_Y_COEFS[9] = {
    0.5,  1, 0.5,
      0,  0,   0,
   -0.5, -1,-0.5
};

const unsigned char MLARR::Analyzer::Coeffs::RGB_HSV[256][3]= {
	{0, 0, 144}, 
	{0, 0, 144}, 
	{0, 0, 144}, 
	{0, 0, 144}, 
	{0, 0, 160}, 
	{0, 0, 160}, 
	{0, 0, 160}, 
	{0, 0, 160}, 
	{0, 0, 176}, 
	{0, 0, 176}, 
	{0, 0, 176}, 
	{0, 0, 176}, 
	{0, 0, 192}, 
	{0, 0, 192}, 
	{0, 0, 192}, 
	{0, 0, 192}, 
	{0, 0, 208}, 
	{0, 0, 208}, 
	{0, 0, 208}, 
	{0, 0, 208}, 
	{0, 0, 224}, 
	{0, 0, 224}, 
	{0, 0, 224}, 
	{0, 0, 224}, 
	{0, 0, 240}, 
	{0, 0, 240}, 
	{0, 0, 240}, 
	{0, 0, 240}, 
	{0, 0, 255}, 
	{0, 0, 255}, 
	{0, 0, 255}, 
	{0, 0, 255}, 
	{0, 16, 255}, 
	{0, 16, 255}, 
	{0, 16, 255}, 
	{0, 16, 255}, 
	{0, 32, 255}, 
	{0, 32, 255}, 
	{0, 32, 255}, 
	{0, 32, 255}, 
	{0, 48, 255}, 
	{0, 48, 255}, 
	{0, 48, 255}, 
	{0, 48, 255}, 
	{0, 64, 255}, 
	{0, 64, 255}, 
	{0, 64, 255}, 
	{0, 64, 255}, 
	{0, 80, 255}, 
	{0, 80, 255}, 
	{0, 80, 255}, 
	{0, 80, 255}, 
	{0, 96, 255}, 
	{0, 96, 255}, 
	{0, 96, 255}, 
	{0, 96, 255}, 
	{0, 112, 255}, 
	{0, 112, 255}, 
	{0, 112, 255}, 
	{0, 112, 255}, 
	{0, 128, 255}, 
	{0, 128, 255}, 
	{0, 128, 255}, 
	{0, 128, 255}, 
	{0, 144, 255}, 
	{0, 144, 255}, 
	{0, 144, 255}, 
	{0, 144, 255}, 
	{0, 160, 255}, 
	{0, 160, 255}, 
	{0, 160, 255}, 
	{0, 160, 255}, 
	{0, 176, 255}, 
	{0, 176, 255}, 
	{0, 176, 255}, 
	{0, 176, 255}, 
	{0, 192, 255}, 
	{0, 192, 255}, 
	{0, 192, 255}, 
	{0, 192, 255}, 
	{0, 208, 255}, 
	{0, 208, 255}, 
	{0, 208, 255}, 
	{0, 208, 255}, 
	{0, 224, 255}, 
	{0, 224, 255}, 
	{0, 224, 255}, 
	{0, 224, 255}, 
	{0, 240, 255}, 
	{0, 240, 255}, 
	{0, 240, 255}, 
	{0, 240, 255}, 
	{0, 255, 255}, 
	{0, 255, 255}, 
	{0, 255, 255}, 
	{0, 255, 255}, 
	{16, 255, 240}, 
	{16, 255, 240}, 
	{16, 255, 240}, 
	{16, 255, 240}, 
	{32, 255, 224}, 
	{32, 255, 224}, 
	{32, 255, 224}, 
	{32, 255, 224}, 
	{48, 255, 208}, 
	{48, 255, 208}, 
	{48, 255, 208}, 
	{48, 255, 208}, 
	{64, 255, 192}, 
	{64, 255, 192}, 
	{64, 255, 192}, 
	{64, 255, 192}, 
	{80, 255, 176}, 
	{80, 255, 176}, 
	{80, 255, 176}, 
	{80, 255, 176}, 
	{96, 255, 160}, 
	{96, 255, 160}, 
	{96, 255, 160}, 
	{96, 255, 160}, 
	{112, 255, 144}, 
	{112, 255, 144}, 
	{112, 255, 144}, 
	{112, 255, 144}, 
	{128, 255, 128}, 
	{128, 255, 128}, 
	{128, 255, 128}, 
	{128, 255, 128}, 
	{144, 255, 112}, 
	{144, 255, 112}, 
	{144, 255, 112}, 
	{144, 255, 112}, 
	{160, 255, 96}, 
	{160, 255, 96}, 
	{160, 255, 96}, 
	{160, 255, 96}, 
	{176, 255, 80}, 
	{176, 255, 80}, 
	{176, 255, 80}, 
	{176, 255, 80}, 
	{192, 255, 64}, 
	{192, 255, 64}, 
	{192, 255, 64}, 
	{192, 255, 64}, 
	{208, 255, 48}, 
	{208, 255, 48}, 
	{208, 255, 48}, 
	{208, 255, 48}, 
	{224, 255, 32}, 
	{224, 255, 32}, 
	{224, 255, 32}, 
	{224, 255, 32}, 
	{240, 255, 16}, 
	{240, 255, 16}, 
	{240, 255, 16}, 
	{240, 255, 16}, 
	{255, 255, 0}, 
	{255, 255, 0}, 
	{255, 255, 0}, 
	{255, 255, 0}, 
	{255, 240, 0}, 
	{255, 240, 0}, 
	{255, 240, 0}, 
	{255, 240, 0}, 
	{255, 224, 0}, 
	{255, 224, 0}, 
	{255, 224, 0}, 
	{255, 224, 0}, 
	{255, 208, 0}, 
	{255, 208, 0}, 
	{255, 208, 0}, 
	{255, 208, 0}, 
	{255, 192, 0}, 
	{255, 192, 0}, 
	{255, 192, 0}, 
	{255, 192, 0}, 
	{255, 176, 0}, 
	{255, 176, 0}, 
	{255, 176, 0}, 
	{255, 176, 0}, 
	{255, 160, 0}, 
	{255, 160, 0}, 
	{255, 160, 0}, 
	{255, 160, 0}, 
	{255, 144, 0}, 
	{255, 144, 0}, 
	{255, 144, 0}, 
	{255, 144, 0}, 
	{255, 128, 0}, 
	{255, 128, 0}, 
	{255, 128, 0}, 
	{255, 128, 0}, 
	{255, 112, 0}, 
	{255, 112, 0}, 
	{255, 112, 0}, 
	{255, 112, 0}, 
	{255, 96, 0}, 
	{255, 96, 0}, 
	{255, 96, 0}, 
	{255, 96, 0}, 
	{255, 80, 0}, 
	{255, 80, 0}, 
	{255, 80, 0}, 
	{255, 80, 0}, 
	{255, 64, 0}, 
	{255, 64, 0}, 
	{255, 64, 0}, 
	{255, 64, 0}, 
	{255, 48, 0}, 
	{255, 48, 0}, 
	{255, 48, 0}, 
	{255, 48, 0}, 
	{255, 32, 0}, 
	{255, 32, 0}, 
	{255, 32, 0}, 
	{255, 32, 0}, 
	{255, 16, 0}, 
	{255, 16, 0}, 
	{255, 16, 0}, 
	{255, 16, 0}, 
	{255, 0, 0}, 
	{255, 0, 0}, 
	{255, 0, 0}, 
	{255, 0, 0}, 
	{240, 0, 0}, 
	{240, 0, 0}, 
	{240, 0, 0}, 
	{240, 0, 0}, 
	{224, 0, 0}, 
	{224, 0, 0}, 
	{224, 0, 0}, 
	{224, 0, 0}, 
	{208, 0, 0}, 
	{208, 0, 0}, 
	{208, 0, 0}, 
	{208, 0, 0}, 
	{192, 0, 0}, 
	{192, 0, 0}, 
	{192, 0, 0}, 
	{192, 0, 0}, 
	{176, 0, 0}, 
	{176, 0, 0}, 
	{176, 0, 0}, 
	{176, 0, 0}, 
	{160, 0, 0}, 
	{160, 0, 0}, 
	{160, 0, 0}, 
	{160, 0, 0}, 
	{144, 0, 0}, 
	{144, 0, 0}, 
	{144, 0, 0}, 
	{144, 0, 0}, 
	{128, 0, 0}, 
	{128, 0, 0}, 
	{128, 0, 0}, 
	{128, 0, 0} 
};
