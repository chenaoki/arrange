//
//  MultiStimEngine.hpp
//  Arrange
//
//  Created by Naoki Tomii on 2014/09/04.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_MultiStimEngine_hpp
#define Arrange_MultiStimEngine_hpp

#include "stdafx.h"

#include "Basic.h"
#include "IO.h"
#include "Electrode.h"
#include "coefs.h"
#include "picojson.h"
#include "Engine.h"
#include "ArduinoSerial.hpp"

using namespace std;
using namespace MLARR::Basic;
using namespace MLARR::IO;
using namespace MLARR::Engine;

namespace MLARR{
	namespace Engine{

	typedef struct state{
	    int numState;
	    int cnt;
	    bool on;
	    bool sw;
	    bool done;
	} state;
                
        
		class MultiStimSetupEngine : public Engine{
        private:
            int numChannel;
            std::string arduinoDeviceFile;
        private:
            vector<map<string, int> > config;
            typedef enum{
                ST_OFF = 0, ST_FLT, ST_POS, ST_NEG
            } enum_state;
        public:
            MultiStimSetupEngine( std::string& paramFilePath) : Engine(paramFilePath){
                
                /* load parameter from file */
                picojson::object obj = MLARR::IO::loadJsonParam(paramFilePath);
                this->numChannel = atoi( obj["numChannel"].to_str().c_str() );
                cout << "numChannel : " << this->numChannel << endl;
                this->arduinoDeviceFile = obj["arduinoDeviceFile"].to_str();
                cout << "arduinoDeviceFile : " << this->arduinoDeviceFile << endl;
                
                map<string,int> m_default;
                m_default["trigger"] = -1;
                m_default["plus"] = 8;
                m_default["minus"] = 2;
                m_default["seq"] = 0;
                m_default["delay"] = 2;
                
                for(int i =0; i < this->numChannel; i++){
                    config.push_back(m_default);
                }
                
                if( ArduinoSerial::Open(arduinoDeviceFile) < 0 ){
                    throw string("failed to open arduino serial port.");
                }
                signal(SIGINT, ArduinoSerial::Interrupt );
                
            };
            
            ~MultiStimSetupEngine(void){
                ArduinoSerial::Close();
            };
            
            void showConfig(void){

                if(config.empty()) return;
                
                cout << "channel ";
                
                for( map<string, int>::iterator it = config[0].begin(); it != config[0].end(); it++ ){
                    string str = it->first;
                    while( str.length() < 8 ) str += string(" ");
                    cout << str;
                }
                cout << endl;
                
                for( vector< map<string, int> >::iterator it_v = config.begin(); it_v != config.end(); it_v++ ){
                    stringstream ss;
                    string str;
                    ss << distance(config.begin(), it_v);
                    ss >> str;
                    while( str.length() < 8 ) str += string(" ");
                    cout << str;
                    for( map<string, int>::iterator it = it_v->begin(); it !=  it_v->end(); it++ ){
                        stringstream ss;
                        string str;
                        ss << it->second;
                        ss >> str;
                        while( str.length() < 8 ) str += string(" ");
                        cout << str;
                    }
                    cout << endl;
                }
                
            };
            
            void send(void){
                
                vector<state> vecState;
                for( int i = 0; i < config.size();i++){
                    state st;
                    st.numState = 0;
                    st.on = false;
                    st.done = ( config[i]["trigger"] >= config.size() || config[i]["trigger"] < -1) ? true : false;
                    st.done = false;
                    st.sw = ( config[i]["seq"] == 0 || config[i]["seq"] == 1) ? false : true;
                    st.cnt = 0;
                    vecState.push_back(st);
                }
                
                bool start = false;
                vector< vector<int> > composition;
                while(1){
                    
                    if( !start ){
                        for( int i = 0; i < config.size();i++) {
                            /* start external trigger channels */
                            if( config[i]["trigger"] == -1 ){
                                start = true;
                                vecState[i].on = true;
                                vecState[i].cnt = ( config[i]["delay"] != 0 ) ? config[i]["delay"] :
                                    ( (config[i]["seq"] % 2 == 0) ? config[i]["plus"] : config[i]["minus"] );
                                vecState[i].numState = ( config[i]["delay"] != 0 ) ? ST_FLT :
                                    ( (config[i]["seq"] % 2 == 0) ? ST_POS : ST_NEG );
                                
                            }else if( config[i]["trigger"] < -1 || config[i]["trigger"] >= config.size() ){
                                vecState[i].on = true;
                                vecState[i].done = true;
                            }
                        }
                        if(!start){
                            throw string("error : no channel was choosed for the beginning of sequence.");
                            break;
                        }
                    }
                    
                    if( start ){
                        
                        /* start internal trigger channels */
                        for( int i = 0; i < config.size();i++) {
                            if( config[i]["trigger"] >= 0 && vecState[i].on == false ){
                                
                                if( vecState[config[i]["trigger"]].done == true){
                                    vecState[i].on = true;
                                    vecState[i].cnt = ( config[i]["delay"] != 0 ) ? config[i]["delay"] :
                                        ( (config[i]["seq"] % 2 == 0) ? config[i]["plus"] : config[i]["minus"] );
                                    vecState[i].numState = ( config[i]["delay"] != 0 ) ? ST_FLT :
                                        ( (config[i]["seq"] % 2 == 0) ? ST_POS : ST_NEG );
                                }
                            }
                        }
                        
                        /* add chord to composition */
                        vector<int> chord;
                        for( int i = 0; i < config.size();i++){
                            int stnum = 0;
                            if( !vecState[i].on || vecState[i].done ){
                                stnum = ST_OFF;
                            }else{
                                stnum = vecState[i].numState;
                            }
                            chord.push_back(stnum);
                        }
                        composition.push_back(chord);


                        /* update state */
                        for( int i = 0; i < config.size();i++) {
                            
                            if( vecState[i].on ){
                                
                                if( !vecState[i].done ){
                                    vecState[i].cnt--;
                                }
                                
                                /* terminate or switch polarity */
                                if( vecState[i].cnt == 0){
                                    if( vecState[i].sw ){
                                        vecState[i].done = true;
                                    }else{
                                        if( ST_FLT == vecState[i].numState ){ /* delay ended */
                                            vecState[i].cnt = (config[i]["seq"] % 2 == 0) ? config[i]["plus"] : config[i]["minus"];
                                            vecState[i].numState = (config[i]["seq"] % 2 == 0) ? ST_POS : ST_NEG ;
                                        }else{
                                            vecState[i].cnt = (config[i]["seq"] % 2 == 0) ? config[i]["minus"] : config[i]["plus"];
                                            vecState[i].numState = (config[i]["seq"] % 2 == 0) ? ST_NEG : ST_POS;
                                            vecState[i].sw = true;
                                        }
                                    }
                                }
                            }
                        }
                        
                        /* termination */
                        bool done = true;
                        for( int i = 0; i < config.size();i++) {
                            done = done && vecState[i].done ;
                        }
                        if( done ) break;

                    }
                    
                }
                
                vector<int> endChord;
                vector<int> lstChord;
                int cnt = 0;
                for( int i = 0; i < config.size(); i++) endChord.push_back(0);
                composition.push_back(endChord);
                
                for( vector< vector<int> >::iterator it_c = composition.begin(); it_c != composition.end(); it_c++){
                    
                    if(it_c == composition.begin()){
                        copy( it_c->begin(), it_c->end(), back_inserter( lstChord ) );
                    }
                    
                    if(*it_c == lstChord )
                        cnt++;
                    
                    if(*it_c != lstChord || it_c + 1 == composition.end()){
                        stringstream ss;
                        string str;
                        string msg(""), resp;
                        for( vector<int>::iterator it_i = lstChord.begin(); it_i != lstChord.end(); it_i++){
                            ss << *it_i; ss >> str; ss.clear();
                            msg += str;
                        }
                        msg += string(" ");
                        ss << cnt; ss >> str; ss.clear();
                        msg += str;
                        cnt = 1;
                        //cout << msg;
                        ArduinoSerial::WriteLine(msg);
                        ArduinoSerial::ReadLine(resp);
                        cout << resp << endl;
                        
                    }
                    
                    lstChord.clear();
                    copy( it_c->begin(), it_c->end(), back_inserter( lstChord ) );
                    if(*it_c == endChord) break;
                    
                }
                return;
            };
            
            void execute(void){
                
                string cmd, resp;
                while(1){
                    bool fin = false;
                    cout << ">>";
                    getline(cin, cmd);
                    list<string> ls;
                    boost::split(ls, cmd, boost::is_space());
                    vector<string> vec(ls.begin(), ls.end());
                    int ch;
                    string cmd = vec[0];
                    
                    switch(vec.size()){
                        case 1:
                            if(cmd == string("c")){
                                cout << "bye" << endl;
                                fin = true;
                            }else if( cmd == string("send")){
                                send();
                            }else if( cmd == string("show")){
                                showConfig();
                            }else if( cmd == string("test")){
                                for(int i = 0; i< config.size(); i++){
                                    config[i]["delay"] = 1000;
                                    config[i]["plus"] = 1000;
                                    config[i]["minus"] = 1000;
                                    config[i]["seq"] = 0;
                                }
                            }else{
                                if( cmd == string("tr") ){
                                    char c = 't';
                                    ArduinoSerial::PutChar(&c);
                                    ArduinoSerial::ReadLine(resp);
                                    cout << resp << endl;
                                }else if(cmd == string("st")){
                                    string msg = cmd;
                                    ArduinoSerial::WriteLine(msg);
                                    ArduinoSerial::ReadLine(resp);
                                    cout << resp << endl;
                                }
                            }
                            break;
                        case 2:
                            if( cmd == string("cw")){
                                ch = atoi( vec[1].c_str() );
                                config[ch]["trigger"] = -1;
                                for( int i = 1; i < config.size(); i++){
                                    int tr = ch;
                                    ch++;
                                    ch = ( ch < config.size() ) ? ch : ch - static_cast<int>(config.size());
                                    config[ch]["trigger"] = tr;
                                }
                            }else if( cmd == string("ccw")){
                                ch = atoi( vec[1].c_str() );
                                config[ch]["trigger"] = -1;
                                for( int i = 1; i < config.size(); i++){
                                    int tr = ch;
                                    ch--;
                                    ch = ( ch >= 0 ) ? ch : ch + static_cast<int>(config.size());
                                    config[ch]["trigger"] = tr;
                                }
                            }
                            break;
                        case 3:
                            if( cmd == string("*")){
                                for(int i = 0; i< config.size(); i++){
                                    config[i][vec[1]] = atoi(vec[2].c_str());
                                }
                            }else{
                                ch = atoi( cmd.c_str() );
                                if( ch >= 0 && ch < config.size() ){
                                    config[ch][vec[1]] = atoi(vec[2].c_str());
                                }
                            }
                            showConfig();
                            break;
                        default:
                            break;
                    }
                    if(fin) break;
                }
            };
        };
        
    }
}


#endif
