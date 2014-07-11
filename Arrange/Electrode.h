#pragma once

#include "stdafx.h"
#include "Basic.h"

#include <boost/algorithm/string.hpp>

namespace MLARR{
	namespace IO{

		class Electrode
		{
		private:
			int id;
			MLARR::Basic::Point<int> imgPos;
			MLARR::Basic::Point<double> axisMajor;
			MLARR::Basic::Point<double> axisMinor;

		public:
			Electrode(int _id, int _posX, int _posY, double majorX, double majorY, double minorX, double minorY) 
				: id(_id), imgPos(_posX, _posY), axisMajor(majorX, majorY), axisMinor(minorX, minorY){};
			virtual ~Electrode(void){};
			
			const int& getID(void){ return id; };
			const MLARR::Basic::Point<int>& getPos(void){ return imgPos; };

			void getFiberCordinates( MLARR::Basic::Point<int>& refPos, MLARR::Basic::Point<double>& cordinates ){

				MLARR::Basic::Point<int> diff_i = refPos - imgPos;
				MLARR::Basic::Point<double> diff_f( diff_i.getX(), diff_i.getY() );
				cordinates = MLARR::Basic::Point<double>( diff_f * axisMajor / axisMajor.abs(), diff_f * axisMinor / axisMinor.abs() );
				return;

			};

		public:
			
			static void loadElectrodeSetting( std::string& path, std::vector<Electrode>& dst ){
				using namespace std;
				string line;
				ifstream ifs(path.c_str());
				if( ifs ){
					while( !ifs.eof() ){
						if( std::getline( ifs, line ) ){
							vector<string> v;
							boost::algorithm::split( v, line, boost::algorithm::is_space() );
							if( v.size() == 7 ){
								dst.push_back( Electrode( 
									atoi(v[0].c_str()), 
									atoi(v[1].c_str()), 
									atoi(v[2].c_str()), 
									atof(v[3].c_str()), 
									atof(v[4].c_str()), 
									atof(v[5].c_str()), 
									atof(v[6].c_str()) 
								));
							}
						}
					}
				}
				return;

			};

		};

	}
}

