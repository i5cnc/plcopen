/*
 * AxisStatus.cpp
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

#include "AxisStatus.hpp"
#include <stdio.h>
namespace Uranus {
    
class AxisStatus::AxisStatusImpl
{
public:
    MC_ErrorCode statusToError(void);
    
    MC_AxisStatus mStatus = MC_AXISSTATUS_DISABLED;
};

MC_ErrorCode AxisStatus::AxisStatusImpl::statusToError(void)
{
    switch(mStatus) {
        case MC_AXISSTATUS_DISABLED:
            return MC_ERRORCODE_AXISDISABLED;
        case MC_AXISSTATUS_STANDSTILL:
            return MC_ERRORCODE_AXISSTANDSTILL;
        case MC_AXISSTATUS_HOMING:
            return MC_ERRORCODE_AXISHOMING;
        case MC_AXISSTATUS_DISCRETEMOTION:
            return MC_ERRORCODE_AXISDISCRETEMOTION;
        case MC_AXISSTATUS_CONTINUOUSMOTION:
            return MC_ERRORCODE_AXISCONTINUOUSMOTION;
        case MC_AXISSTATUS_SYNCHRONIZEDMOTION:
            return MC_ERRORCODE_AXISSYNCHRONIZEDMOTION;
        case MC_AXISSTATUS_STOPPING:
            return MC_ERRORCODE_AXISSTOPPING;
        case MC_AXISSTATUS_ERRORSTOP:
            return MC_ERRORCODE_AXISERRORSTOP;
    }
    
    return MC_ERRORCODE_AXISDISABLED;
}

AxisStatus::AxisStatus()
{
    mImpl_ = new AxisStatusImpl();
    URANUS_ADD_HANDLER(onError, onErrorHandler);
    URANUS_ADD_HANDLER(onPowerStatusChanged, onPowerStatusChangedHandler);
}

AxisStatus::~AxisStatus()
{
    delete mImpl_;
}

MC_AxisStatus AxisStatus::status(void)
{
    return mImpl_->mStatus;
}

MC_ErrorCode AxisStatus::testStatus(MC_AxisStatus status)
{
    if(status == mImpl_->mStatus)
        return MC_ERRORCODE_GOOD;
    
    switch(mImpl_->mStatus) {
        case MC_AXISSTATUS_ERRORSTOP:
        case MC_AXISSTATUS_DISABLED:
            goto FAILED;
            
        case MC_AXISSTATUS_STANDSTILL:
            switch(status) {
                case MC_AXISSTATUS_DISCRETEMOTION:
                case MC_AXISSTATUS_CONTINUOUSMOTION:
                case MC_AXISSTATUS_SYNCHRONIZEDMOTION:
                case MC_AXISSTATUS_STOPPING:
                case MC_AXISSTATUS_HOMING:
                    goto SUCCESS;
                default:
                    goto FAILED;
            }
            
        case MC_AXISSTATUS_DISCRETEMOTION:
        case MC_AXISSTATUS_CONTINUOUSMOTION:
        case MC_AXISSTATUS_SYNCHRONIZEDMOTION:
            switch(status) {
                case MC_AXISSTATUS_DISCRETEMOTION:
                case MC_AXISSTATUS_CONTINUOUSMOTION:
                case MC_AXISSTATUS_SYNCHRONIZEDMOTION:
                case MC_AXISSTATUS_STANDSTILL:
                case MC_AXISSTATUS_STOPPING:
                    goto SUCCESS;
                default:
                    goto FAILED;
            }
            
        case MC_AXISSTATUS_HOMING:
            switch(status) {
                case MC_AXISSTATUS_STANDSTILL:
                case MC_AXISSTATUS_STOPPING:
                    goto SUCCESS;
                default:
                    goto FAILED;
            }
            
        case MC_AXISSTATUS_STOPPING:
            switch(status) {
                case MC_AXISSTATUS_STANDSTILL:
                    goto SUCCESS;
                default:
                    goto FAILED;
            }
            
        default:
            goto FAILED;
    }
    
SUCCESS:
    return MC_ERRORCODE_GOOD;
    
FAILED:
    return mImpl_->statusToError();
}

MC_ErrorCode AxisStatus::setStatus(MC_AxisStatus status)
{
    MC_ErrorCode err = testStatus(status);
    if(err) return err;
    
    mImpl_->mStatus = status;
    return MC_ERRORCODE_GOOD;
}

void AxisStatus::onErrorHandler(AxisBase* this_, MC_ErrorCode errorCode)
{
    AxisStatus* this__ = dynamic_cast<AxisStatus*>(this_);
    this__->mImpl_->mStatus = MC_AXISSTATUS_ERRORSTOP;
}

void AxisStatus::onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus)
{
    AxisStatus* this__ = dynamic_cast<AxisStatus*>(this_);
    this__->mImpl_->mStatus = 
        powerStatus? MC_AXISSTATUS_STANDSTILL: MC_AXISSTATUS_DISABLED;
}

}
