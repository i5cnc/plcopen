/*
 * FbPLCOpenBase.cpp
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

#include "FbPLCOpenBase.hpp"

namespace Uranus {
    
void FbBaseType::onOperationError(MC_ErrorCode errorCode, int32_t customId)
{
    mError = true;
    mErrorID = errorCode;
}

void FbBaseType::clearError(void)
{
    mError = false;
    mErrorID = MC_ERRORCODE_GOOD;
}

////////////////////////////////////////////////////////////

void FbEnableType::call(void)
{
    MC_ErrorCode err = mEnable? onEnableTrue(): onEnableFalse();
    if(err) 
        onOperationError(err, 0);
    else
        clearError();
}

////////////////////////////////////////////////////////////

void FbComExecuteType::call(void)
{
    if(mExecute) {
        if(mDone || mError)
            return;
        
        bool isDone = false;
        MC_ErrorCode err = onExecTriggered(isDone);
        if(err) {
            onOperationError(err, 0);
            return;
        }
        
        if(isDone) {
            mDone = true;
            mBusy = false;
        } else {
            mDone = false;
            mBusy = true;
        }
    } else {
        mDone = false;
        mBusy = false;
    }
    
    clearError();
}

void FbComExecuteType::onOperationError(MC_ErrorCode errorCode, int32_t customId)
{
    mDone = mBusy = false;
    FbBaseType::onOperationError(errorCode, customId);
}

////////////////////////////////////////////////////////////

void FbSeqExecuteType::call(void)
{
    if(mExecute && !mExecuteTrigger) { //上升沿
        MC_ErrorCode err = onExecPosedge();
        if(err) {
            onOperationError(err, 0);
        } else {
            mDone = mActive = mCommandAborted = false;
            mBusy = true;
            mOkFlag = false;
            clearError();
        }
    } else if(mExecute && mExecuteTrigger) { //高值
        
    } else if(!mExecute && mExecuteTrigger) { //下降沿
        if((mDone || mCommandAborted || mError) && !mBusy) {
            mDone = mCommandAborted = mBusy = mActive = false;
            clearError();
        } 
        mOkFlag = false;
        onExecNegedge();
    } else { //低值
        if(!mBusy && !mOkFlag) {
            mDone = mCommandAborted = mBusy = mActive = false;
            clearError();
        }
        mOkFlag = false;
    }
    
    mExecuteTrigger = mExecute;
}

void FbSeqExecuteType::onOperationActive(int32_t customId)
{
    mDone = mCommandAborted = false;
    mBusy = mActive = true;
    clearError();
}

void FbSeqExecuteType::onOperationAborted(int32_t customId)
{
    mDone = mBusy = mActive = false;
    mCommandAborted = true;
    mOkFlag = !mExecute;
    clearError();
}

void FbSeqExecuteType::onOperationDone(int32_t customId)
{
    mCommandAborted = mBusy = mActive = false;
    mDone = true;
    mOkFlag = !mExecute;
    clearError();
}

void FbSeqExecuteType::onOperationError(MC_ErrorCode errorCode, int32_t customId)
{
    mDone = mBusy = mActive = mCommandAborted = false;
    mOkFlag = !mExecute;
    FbBaseType::onOperationError(errorCode, customId);
}

////////////////////////////////////////////////////////////

MC_ErrorCode FbReadInfoType::onEnableTrue(void)
{
    bool isDone = false;
    MC_ErrorCode err = onEnable(isDone);
    if(err) return err;
    mValid = isDone;
    mBusy = !isDone;
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode FbReadInfoType::onEnableFalse(void)
{
    mValid = false;
    mBusy = false;
    onDisable();
    return MC_ERRORCODE_GOOD;
}

void FbReadInfoType::onOperationError(MC_ErrorCode errorCode, int32_t customId)
{
    mValid = false;
    mBusy = false;
    onDisable();
    FbBaseType::onOperationError(errorCode, customId);
}

////////////////////////////////////////////////////////////

MC_ErrorCode FbExecAxisType::onExecPosedge(void)
{
    return mAxis? onAxisExecPosedge(): MC_ERRORCODE_AXISNOTEXIST;
}

////////////////////////////////////////////////////////////

void FbExecAxisBufferContType::onOperationDone(int32_t customId)
{
    mCommandAborted = false;
    mBusy = mActive = mDone = true;
    clearError();
}

////////////////////////////////////////////////////////////

MC_ErrorCode FbReadInfoAxisType::onEnable(bool& isDone)
{
    return mAxis? onAxisEnable(isDone): MC_ERRORCODE_AXISNOTEXIST;
}

////////////////////////////////////////////////////////////

MC_ErrorCode FbWriteInfoAxisType::onExecTriggered(bool& isDone)
{
    return mAxis? onAxisTriggered(isDone): MC_ERRORCODE_AXISNOTEXIST;
}

////////////////////////////////////////////////////////////

MC_ErrorCode FbExecAxisBufferContSyncType::onAxisExecPosedge(void)
{
    return mMaster? onMasterSlaveExecPosedge(): MC_ERRORCODE_AXISNOTEXIST;
}

////////////////////////////////////////////////////////////


}
