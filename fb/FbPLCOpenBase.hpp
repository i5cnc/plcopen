/*
 * FbPLCOpenBase.hpp
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

#ifndef _URANUS_FBPLCOPENBASE_HPP_
#define _URANUS_FBPLCOPENBASE_HPP_

#include "FunctionBlock.hpp"
#include "PLCTypes.hpp"

namespace Uranus {
    
#pragma pack(push)
#pragma pack(4)

#define FB_INPUT 
#define FB_OUTPUT 

class FbBaseType : virtual public FunctionBlock
{
public:
    FB_OUTPUT BOOL mError = false;
    FB_OUTPUT MC_ERRORCODE mErrorID = MC_ERRORCODE_GOOD;
    
public:
    void onOperationError(MC_ErrorCode errorCode, int32_t customId);
        
    void clearError(void);
       
public: 
    virtual void call(void) = 0;
};

class FbEnableType : virtual public FbBaseType
{
public:
    FB_INPUT BOOL mEnable = false;
    
public:
    void call(void);
    
public:
    virtual MC_ErrorCode onEnableTrue(void) = 0;
    virtual MC_ErrorCode onEnableFalse(void) = 0;
};

class FbComExecuteType : virtual public FbBaseType
{
public:
    FB_INPUT BOOL mExecute = false;
    
    FB_OUTPUT BOOL mDone = false;
    FB_OUTPUT BOOL mBusy = false;
    
public:
    void call(void);
    
    void onOperationError(MC_ErrorCode errorCode, int32_t customId);
    
public:
    virtual MC_ErrorCode onExecTriggered(bool& isDone) = 0;
};

class FbSeqExecuteType : virtual public FbBaseType
{
public:
    FB_INPUT BOOL mExecute = false;
    
    FB_OUTPUT BOOL mDone = false;
    FB_OUTPUT BOOL mBusy = false;
    FB_OUTPUT BOOL mActive = false;
    FB_OUTPUT BOOL mCommandAborted = false;
    
public:
    void call(void);
    
    void onOperationActive(int32_t customId);
    
    void onOperationAborted(int32_t customId);
    
    void onOperationDone(int32_t customId);
    
    void onOperationError(MC_ErrorCode errorCode, int32_t customId);
    
public:
    virtual MC_ErrorCode onExecPosedge(void) = 0;
    virtual void onExecNegedge(void){}
    
private:
    bool mExecuteTrigger = false;
    bool mOkFlag = false;
};

class FbReadInfoType : virtual public FbEnableType
{
public:
    FB_OUTPUT BOOL mValid = false;
    FB_OUTPUT BOOL mBusy = false;
    
public:
    MC_ErrorCode onEnableTrue(void);
    MC_ErrorCode onEnableFalse(void);
    
    void onOperationError(MC_ErrorCode errorCode, int32_t customId);
        
public:
    virtual MC_ErrorCode onEnable(bool& isDone) = 0;
    virtual void onDisable(void) = 0;
};

class FbWriteInfoAxisType : virtual public FbComExecuteType
{
public:
    FB_INPUT AXIS_REF mAxis = nullptr;
    
public:
    MC_ErrorCode onExecTriggered(bool& isDone);

public:
    virtual MC_ErrorCode onAxisTriggered(bool& isDone) = 0;
};

class FbReadInfoAxisType : virtual public FbReadInfoType
{
public:
    FB_INPUT AXIS_REF mAxis = nullptr;
    
public:
    MC_ErrorCode onEnable(bool& isDone);
        
public:
    virtual MC_ErrorCode onAxisEnable(bool& isDone) = 0;
};

class FbCoordSystemType
{
public:
    FB_INPUT MC_COORD_SYSTEM mCoordSystem = MC_COORDSYSTEM_MCS;
};

class FbBufferModeType : virtual public FbSeqExecuteType
{
public:
    FB_INPUT MC_BUFFER_MODE mBufferMode = MC_BUFFERMODE_ABORTING;
};

class FbTranslModeType : virtual public FbSeqExecuteType
{
public:
    FB_INPUT MC_TRANSITION_MODE mTransitionMode = MC_TRANSITIONMODE_NONE;
    FB_INPUT LREAL mTransitionParameter[URANUS_TRANSITIONPARAMETER_NUM] = {0};
};

class FbExecAxisType : virtual public FbSeqExecuteType
{
public:
    FB_INPUT AXIS_REF mAxis = nullptr;
    
public:
    MC_ErrorCode onExecPosedge(void);
    
public:
    virtual MC_ErrorCode onAxisExecPosedge(void) = 0;
};

class FbExecAxisBufferType : 
    virtual public FbExecAxisType, virtual public FbBufferModeType
{
};

class FbExecAxisBufferContType : virtual public FbExecAxisBufferType
{
public:
    void onOperationDone(int32_t customId);
};

class FbExecAxisBufferContSyncType : virtual public FbExecAxisBufferContType
{
public:
    FB_INPUT AXIS_REF mMaster = nullptr;
    FB_INPUT AXIS_REF& mSlave = mAxis;
    FB_OUTPUT BOOL mStartSync = false;
    FB_OUTPUT BOOL& mInGear = mDone;
    FB_OUTPUT BOOL& mInSync = mDone;
    
public:
    MC_ErrorCode onAxisExecPosedge(void);
    
public:
    virtual MC_ErrorCode onMasterSlaveExecPosedge(void) = 0;
};

#pragma pack(pop)

}

#endif /** _URANUS_FBPLCOPENBASE_HPP_ **/
    
