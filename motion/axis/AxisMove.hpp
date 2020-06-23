/*
 * AxisMove.hpp
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

#ifndef _URANUS_AXISMOVE_HPP_
#define _URANUS_AXISMOVE_HPP_

#include "AxisMotionBase.hpp"

namespace Uranus {
    
class AxisMove : virtual public AxisMotionBase
{
public:
    AxisMove();
    virtual ~AxisMove();
    
    MC_ErrorCode addMovePos(
        FunctionBlock* fb, 
        double pos, 
        double vel, 
        double acc, 
        double dec, 
        double jerk, 
        MC_ShiftingMode shiftingMode = MC_SHIFTINGMODE_ABSOLUTE,
        MC_Direction dir = MC_DIRECTION_CURRENT, 
        MC_BufferMode bufferMode = MC_BUFFERMODE_ABORTING,
        int32_t customId = 0);
        
    MC_ErrorCode addMovePosCont(
        FunctionBlock* fb, 
        double pos, 
        double vel, 
        double acc, 
        double dec, 
        double endVel, 
        double jerk, 
        MC_ShiftingMode shiftingMode = MC_SHIFTINGMODE_ABSOLUTE,
        MC_Direction dir = MC_DIRECTION_CURRENT, 
        MC_BufferMode bufferMode = MC_BUFFERMODE_ABORTING,
        int32_t customId = 0);
        
    MC_ErrorCode addMoveVel(
        FunctionBlock* fb, 
        double vel, 
        double acc, 
        double dec, 
        double jerk, 
        MC_BufferMode bufferMode = MC_BUFFERMODE_ABORTING,
        int32_t customId = 0);
        
    MC_ErrorCode addHalt(
        FunctionBlock* fb, 
        double dec, 
        double jerk, 
        MC_BufferMode bufferMode = MC_BUFFERMODE_ABORTING,
        int32_t customId = 0);
        
    MC_ErrorCode addStop(
        FunctionBlock* fb, 
        double dec, 
        double jerk,
        int32_t customId = 0);
        
    void cancelStopLater(void);
    
private:
    static void onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus);
    static void onPositionOffsetHandler(AxisBase* this_, double positionOffset);
    static void onAllNodesAbortedHandler(ExeclQueue* this_);
    static void onAllNodesErrorHandler(ExeclQueue* this_, MC_ErrorCode errorCodeToSet);
    
private:
    class AxisMoveImpl;
    AxisMoveImpl* mImpl_;
    friend class MoveNode;
};

}
#endif /** _URANUS_AXISMOVE_HPP_ **/
