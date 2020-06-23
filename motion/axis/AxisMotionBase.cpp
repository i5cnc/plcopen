/*
 * AxisMotionBase.cpp
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

#include "AxisMotionBase.hpp"
#include "FunctionBlock.hpp"
//#include "AxesGroupBase.hpp"

namespace Uranus {
    
class AxisMotionBase::AxisMotionBaseImpl
{
public:
    //AxesGroupBase* mGroup = nullptr;
};

MC_ErrorCode AxisExeclNode::onActive(ExeclQueue* queue)
{
    AxisMotionBase* axis = dynamic_cast<AxisMotionBase*>(queue);
    MC_ErrorCode err = axis->setStatus(mStatusActive);
    if(err) return err;
    
    axis->operationActive(mFb, mNodeCustomId);
    if(mFb) 
        mFb->onOperationActive(mNodeCustomId);
    
    return MC_ERRORCODE_GOOD;
}

void AxisExeclNode::onAborted(ExeclQueue* queue)
{
    AxisMotionBase* axis = dynamic_cast<AxisMotionBase*>(queue);
    axis->operationAborted(mFb, mNodeCustomId);
    if(mFb)
        mFb->onOperationAborted(mNodeCustomId);
}

void AxisExeclNode::onDone(ExeclQueue* queue, bool& isHold)
{
    AxisMotionBase* axis = dynamic_cast<AxisMotionBase*>(queue);
    axis->setStatus(mStatusDone);
    axis->operationDone(mFb, mNodeCustomId);
    if(mFb)
        mFb->onOperationDone(mNodeCustomId);
}

void AxisExeclNode::onError(ExeclQueue* queue, MC_ErrorCode errorCode)
{
    AxisMotionBase* axis = dynamic_cast<AxisMotionBase*>(queue);
    axis->operationError(mFb, mNodeCustomId, errorCode);
    if(mFb)
        mFb->onOperationError(errorCode, mNodeCustomId);
}

AxisMotionBase::AxisMotionBase()
{
    mImpl_ = new AxisMotionBaseImpl();
    URANUS_ADD_HANDLER(onError, onErrorHandler);
    URANUS_ADD_HANDLER(onPowerStatusChanged, onPowerStatusChangedHandler);
    URANUS_ADD_HANDLER(onPositionOffset, onPositionOffsetHandler);
}

AxisMotionBase::~AxisMotionBase()
{
    delete mImpl_;
}

void AxisMotionBase::runCycle(void)
{
    processExeclNode();
    AxisBase::runCycle();
}

MC_ErrorCode AxisMotionBase::pushAndNewData(
    const std::function<AxisExeclNode*(void*)>& constructor,
    bool abortFlag, 
    FunctionBlock* fb,
    MC_AxisStatus statusActive,
    MC_AxisStatus statusDone,
    int32_t nodeCustomId)
{
    if(errorCode())
        return errorCode();
    
    if(!powerStatus())
        return MC_ERRORCODE_AXISPOWEROFF;
        
    if(abortFlag) {
        MC_ErrorCode err = setStatus(statusActive);
        if(err) return err;
    }
    
    return ExeclQueue::pushAndNewData(
        [&constructor, fb, statusActive, statusDone, nodeCustomId]
        (void* baseNode) -> AxisExeclNode* {
            AxisExeclNode* node = constructor(baseNode);
            node->mFb = fb;
            node->mStatusActive = statusActive;
            node->mStatusDone = statusDone;
            node->mNodeCustomId = nodeCustomId;
            return node;
        }, abortFlag);
}

void AxisMotionBase::onErrorHandler(AxisBase* this_, MC_ErrorCode errorCode)
{
    AxisMotionBase* this__ = dynamic_cast<AxisMotionBase*>(this_);
    this__->setAllNodesError(errorCode);
}

void AxisMotionBase::onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus)
{
    AxisMotionBase* this__ = dynamic_cast<AxisMotionBase*>(this_);
    this__->setAllNodesAborted();
    /*
    AxesGroupBase* group = this__->mImpl_->mGroup;
    if(group) {
        URANUS_CALL_EVENT(group->onAxisPowerStatusChanged, group, this__, powerStatus);
    }
    */
}

void AxisMotionBase::onPositionOffsetHandler(AxisBase* this_, double positionOffset)
{
    AxisMotionBase* this__ = dynamic_cast<AxisMotionBase*>(this_);
    
    AxisExeclNode* node = 
        dynamic_cast<AxisExeclNode*>(this__->ExeclQueue::front());
    
    while(node) {
        node->onPositionOffset(this__, positionOffset);
        node = dynamic_cast<AxisExeclNode*>(this__->ExeclQueue::next(node));
    }
}

}
