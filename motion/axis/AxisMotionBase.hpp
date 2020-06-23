/*
 * AxisMotionBase.hpp
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

#ifndef _URANUS_AXISMOTIONBASE_HPP_
#define _URANUS_AXISMOTIONBASE_HPP_

#include "AxisStatus.hpp"
#include "ExeclQueue.hpp"
#include "LinkList.hpp"

namespace Uranus {

//class AxesGroupBase;
class FunctionBlock;
class AxisExeclNode : virtual public ExeclNode
{
public:
    FunctionBlock* mFb = nullptr;
    MC_AxisStatus mStatusActive = MC_AXISSTATUS_STANDSTILL;
    MC_AxisStatus mStatusDone = MC_AXISSTATUS_STANDSTILL;
    int32_t mNodeCustomId = 0;

protected:
    virtual MC_ErrorCode onActive(ExeclQueue* queue) override;
    virtual void onAborted(ExeclQueue* queue) override;
    virtual void onDone(ExeclQueue* queue, bool& isHold) override;
    virtual void onError(ExeclQueue* queue, MC_ErrorCode errorCode) override;
    
public:
    virtual void onPositionOffset(ExeclQueue* queue, double positionOffset) = 0;
};

class AxisMotionBase : 
    virtual public AxisStatus, 
    virtual public ExeclQueue,
    public LinkNode
{
public:
    AxisMotionBase();
    virtual ~AxisMotionBase();
    void runCycle(void);
    MC_ErrorCode pushAndNewData(
        const std::function<AxisExeclNode*(void*)>& constructor,
        bool abortFlag, 
        FunctionBlock* fb,
        MC_AxisStatus statusActive,
        MC_AxisStatus statusDone,
        int32_t nodeCustomId);

public: //外部继承获取
    virtual void operationActive(FunctionBlock* fb, int32_t customId){}
    virtual void operationAborted(FunctionBlock* fb, int32_t customId){}
    virtual void operationDone(FunctionBlock* fb, int32_t customId){}
    virtual void operationError(
        FunctionBlock* fb, int32_t customId, MC_ErrorCode errorCode){}

private:
    static void onErrorHandler(AxisBase* this_, MC_ErrorCode errorCode);
    static void onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus);
    static void onPositionOffsetHandler(AxisBase* this_, double positionOffset);

private:
    class AxisMotionBaseImpl;
    AxisMotionBaseImpl* mImpl_;
};

}

#endif /** _URANUS_AXISMOTIONBASE_HPP_ **/
