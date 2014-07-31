#pragma once

#include "stdafx.h"
#include "Basic.h"

#include <boost/algorithm/string.hpp>

namespace MLARR{
	namespace IO{

		class Electrode : public MLARR::Basic::Point<int>
		{
		private:
			int id;

		public:
			Electrode(int _id, int _posX, int _posY)
            : MLARR::Basic::Point<int>(_posX, _posY), id(_id){};
			virtual ~Electrode(void){};
			
			const int& getID(void){ return id; };

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
									atoi(v[2].c_str())
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

