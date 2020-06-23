/*
 * FbSingleAxis.hpp
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

#ifndef _URANUS_FBSINGLEAXIS_HPP_
#define _URANUS_FBSINGLEAXIS_HPP_

#include "FbPLCOpenBase.hpp"

namespace Uranus {
    
#pragma pack(push)
#pragma pack(4)

class FbPower : public FbBaseType
{
public:
    FB_INPUT AXIS_REF mAxis;

    FB_INPUT BOOL mEnable = false;
    FB_INPUT BOOL mEnablePositive = false;
    FB_INPUT BOOL mEnableNegative = false;
    
    FB_OUTPUT BOOL mStatus = false;
    FB_OUTPUT BOOL mValid = false;
    
public:
    void call(void);
    
    void onOperationError(MC_ErrorCode errorCode, int32_t customId);
};

class FbHome : public FbExecAxisBufferType
{
public:
    FB_INPUT LREAL mPosition = 0;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
};

class FbStop : public FbExecAxisType
{
public:
    FB_INPUT LREAL mDeceleration = 0;
    FB_INPUT LREAL mJerk = 0;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
    void onExecNegedge(void);
};

class FbHalt : public FbExecAxisBufferType
{
public:
    FB_INPUT LREAL mDeceleration = 0;
    FB_INPUT LREAL mJerk = 0;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
};

class FbMoveAbsolute : public FbExecAxisBufferType
{
public:
    FB_INPUT LREAL mPosition = 0;
    FB_INPUT LREAL mVelocity = 0;
    FB_INPUT LREAL mAcceleration = 0;
    FB_INPUT LREAL mDeceleration = 0;
    FB_INPUT LREAL mJerk = 0;
    FB_INPUT MC_DIRECTION mDirection = MC_DIRECTION_CURRENT;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
};

class FbMoveRelative : public FbExecAxisBufferType
{
public:
    FB_INPUT LREAL mDistance = 0;
    FB_INPUT LREAL mVelocity = 0;
    FB_INPUT LREAL mAcceleration = 0;
    FB_INPUT LREAL mDeceleration = 0;
    FB_INPUT LREAL mJerk = 0;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
};

class FbMoveAdditive : public FbExecAxisBufferType
{
public:
    FB_INPUT LREAL mDistance = 0;
    FB_INPUT LREAL mVelocity = 0;
    FB_INPUT LREAL mAcceleration = 0;
    FB_INPUT LREAL mDeceleration = 0;
    FB_INPUT LREAL mJerk = 0;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
};

class FbMoveVelocity : public FbExecAxisBufferContType
{
public:
    FB_INPUT LREAL mVelocity = 0;
    FB_INPUT LREAL mAcceleration = 0;
    FB_INPUT LREAL mDeceleration = 0;
    FB_INPUT LREAL mJerk = 0;
    
    FB_OUTPUT BOOL& mInVelocity = mDone;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
};

class FbReadStatus : public FbReadInfoAxisType
{
public:
    FB_OUTPUT BOOL mErrorStop = false;
    FB_OUTPUT BOOL mDisabled = false;
    FB_OUTPUT BOOL mStopping = false;
    FB_OUTPUT BOOL mHoming = false;
    FB_OUTPUT BOOL mStandstill = false;
    FB_OUTPUT BOOL mDiscreteMotion = false;
    FB_OUTPUT BOOL mContinuousMotion = false;
    FB_OUTPUT BOOL mSynchronizedMotion = false;
    
public:
    MC_ErrorCode onAxisEnable(bool& isDone);
    void onDisable(void);
};

class FbReadMotionState : public FbReadInfoAxisType
{
public:
    FB_INPUT MC_SOURCE mSource = MC_SOURCE_SETVALUE;
    
    FB_OUTPUT BOOL mConstantVelocity = false;
    FB_OUTPUT BOOL mAccelerating = false;
    FB_OUTPUT BOOL mDecelerating = false;
    FB_OUTPUT BOOL mDirectionPositive = false;
    FB_OUTPUT BOOL mDirectionNegative = false;
    
public:
    MC_ErrorCode onAxisEnable(bool& isDone);
    void onDisable(void);
};

class FbReadAxisError : public FbEnableType
{
public:
    FB_INPUT AXIS_REF mAxis = nullptr;
    FB_OUTPUT BOOL mValid = false;
    FB_OUTPUT BOOL mBusy = false;
    FB_OUTPUT MC_SERVOERRORCODE mAxisErrorID = 0;
    
public:
    void call(void);
    MC_ErrorCode onEnableTrue(void);
    MC_ErrorCode onEnableFalse(void);
};

class FbReset : public FbWriteInfoAxisType
{
public:
    MC_ErrorCode onAxisTriggered(bool& isDone);
};

typedef enum {
    MC_PARAMETER_COMMANDEDPOSITION      = 1,
    MC_PARAMETER_SWLIMITPOS             = 2,
    MC_PARAMETER_SWLIMITNEG             = 3,
    MC_PARAMETER_ENABLELIMITPOS         = 4,
    MC_PARAMETER_ENABLELIMITNEG         = 5,
    MC_PARAMETER_ENABLEPOSLAGMONITORING = 6,
    MC_PARAMETER_MAXPOSITIONLAG         = 7,
    MC_PARAMETER_MAXVELOCITYSYSTEM      = 8,
    MC_PARAMETER_MAXVELOCITYAPPL        = 9,
    MC_PARAMETER_ACTUALVELOCITY         = 10,
    MC_PARAMETER_COMMANDEDVELOCITY      = 11,
    MC_PARAMETER_MAXACCELERATIONSYSTEM  = 12,
    MC_PARAMETER_MAXACCELERATIONAPPL    = 13,
    MC_PARAMETER_MAXDECELERATIONSYSTEM  = 14,
    MC_PARAMETER_MAXDECELERATIONAPPL    = 15,
    MC_PARAMETER_MAXJERKSYSTEM          = 16,
    MC_PARAMETER_MAXJERKAPPL            = 17,
}MC_Parameter;

class FbReadActualPosition : public FbReadInfoAxisType
{
public:
    FB_OUTPUT LREAL mPosition = 0;
    
public:
    MC_ErrorCode onAxisEnable(bool& isDone);
    void onDisable(void);
};

class FbReadCommandPosition : public FbReadActualPosition
{
public:
    MC_ErrorCode onAxisEnable(bool& isDone);
};

class FbReadActualVelocity : public FbReadInfoAxisType
{
public:
    FB_OUTPUT LREAL mVelocity = 0;
    
public:
    MC_ErrorCode onAxisEnable(bool& isDone);
    void onDisable(void);
};

class FbReadCommandVelocity : public FbReadActualVelocity
{
public:
    MC_ErrorCode onAxisEnable(bool& isDone);
};

class FbEmergencyStop : public FbWriteInfoAxisType
{
public:
    MC_ErrorCode onAxisTriggered(bool& isDone);
};

#pragma pack(pop)

}

#endif /** _URANUS_FBSINGLEAXIS_HPP_ **/
    
