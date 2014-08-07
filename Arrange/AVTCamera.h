//
//  AVTCamera.h
//  Arrange
//
//  Created by Naoki Tomii on 2014/08/07.
//  Copyright (c) 2014年 ARR. All rights reserved.
//

#ifndef Arrange_AVTCamera_h
#define Arrange_AVTCamera_h

#include "IO.h"

namespace MLARR {
    
    namespace IO {
    
        class AVTCamera : public ICamera<unsigned short>{
        public:
            AVTCamera( const std::string& paramFilePath);
            ~AVTCamera(void);
        };
    }
}

#endif
