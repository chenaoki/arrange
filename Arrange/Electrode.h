#pragma once

#include "stdafx.h"
#include "Basic.h"

#include <boost/algorithm/string.hpp>

namespace MLARR{
	namespace IO{

		class Electrode : public MLARR::Basic::Point<int>
		{
		protected:
			int id;

		public:
			Electrode(int _id, int _posX, int _posY)
            : MLARR::Basic::Point<int>(_posX, _posY), id(_id){};
			virtual ~Electrode(void){};
			
		public:
			int getID(void) const{ return id; };
            
		};
        
        class ActivationMonitorElectrode : public Electrode
        {
        protected:
            int lastPastTime;
            int pastTime;
            const int minPeakDist;
            double filVal;
            double thre;
            std::vector<double> actBuf;
            std::vector<double> vecCoeff;

        public:
			ActivationMonitorElectrode(int _id, int _posX, int _posY, std::vector<double> _vecCoeff, double threshold, int minPeakDistance )
            : Electrode(_id, _posX, _posY), lastPastTime(-minPeakDistance), pastTime(0), minPeakDist(minPeakDistance), thre(threshold), filVal(0.0), vecCoeff(_vecCoeff), actBuf(){};
            
            ActivationMonitorElectrode( const Electrode& elec, std::vector<double> _vecCoeff, double threshold, int minPeakDistance )
            : Electrode( elec.getID(), elec.getX(), elec.getY()), lastPastTime(-minPeakDistance), pastTime(0), minPeakDist(minPeakDistance), thre(threshold), filVal(0.0), vecCoeff(_vecCoeff), actBuf(){};
            
			virtual ~ActivationMonitorElectrode(void){};
			
        public:
			int getPastTime(void){ return pastTime; };
            double getFilteredValue(void){ return filVal; }
            void monitor(int msec, double actValue)
            {
                actBuf.push_back(actValue);
                while( actBuf.size() > vecCoeff.size() ){
                    actBuf.erase(actBuf.begin());
                }
                filVal = 0.0;
                if(  actBuf.size() == vecCoeff.size() ){
                    for( int i = 0; i < actBuf.size(); i++){
                        filVal += actBuf[i] * vecCoeff[i];
                    }
                }
                if( filVal > thre && msec > lastPastTime + minPeakDist){
                    lastPastTime = msec;
                }
                if( lastPastTime > 0 ) pastTime = msec - lastPastTime;
                return;
            };
            
        };

	}
}

