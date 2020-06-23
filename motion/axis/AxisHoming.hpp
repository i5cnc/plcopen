/*
 * AxisHoming.hpp
 * 
 * Copyright 2020 (C) SYMG(Shanghai) Intelligence System Co.,Ltd
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 */

#ifndef _URANUS_AXISHOMING_HPP_
#define _URANUS_AXISHOMING_HPP_

#include "AxisMotionBase.hpp"

namespace Uranus {
    
class AxisHoming : virtual public AxisMotionBase
{
public:
    AxisHoming();
    virtual ~AxisHoming();
    
public:
    MC_ErrorCode setHomingInfo(const AxisHomingInfo& info);
    
    MC_ErrorCode addHoming(
        FunctionBlock* fb, 
        double pos, 
        MC_BufferMode bufferMode = MC_BUFFERMODE_ABORTING,
        int32_t customId = 0);
       
private: 
    static void onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus);
    static void onPositionOffsetHandler(AxisBase* this_, double positionOffset);
    
private:
    class AxisHomingImpl;
    AxisHomingImpl* mImpl_;
    friend class HomingNode;
};

}
#endif /** _URANUS_AXISHOMING_HPP_ **/
