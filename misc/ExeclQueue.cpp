/*
 * ExeclQueue.cpp
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
 
#include "ExeclQueue.hpp"
#include "Queue.hpp"

#include <cstdint>

namespace Uranus {
    
#define URANUS_AXISEXECLLISTSIZE 6
#define URANUS_AXISEXECLNODESIZE 1024

struct ExeclNodeContainer
{
    ExeclNode* node = nullptr;
    uint8_t data[URANUS_AXISEXECLNODESIZE];
};

ExeclNode::ExeclNode()
{
}

ExeclNode::~ExeclNode()
{
}

class ExeclQueue::ExeclQueueImpl
{
public:
    ExeclQueue* mThis_ = nullptr;;
    Queue<ExeclNodeContainer, URANUS_AXISEXECLLISTSIZE> mQueue;
    ExeclNode* mHoldNode = nullptr;
    
public:
    void processFrontNode(void);
};

void ExeclQueue::ExeclQueueImpl::processFrontNode(void)
{
    MC_ErrorCode err;
    ExeclNodeExecStat stat = EXECLNODEEXECSTAT_BUSY;
    ExeclNodeContainer* container = mQueue.front();
    if(!container)
        return;
        
    ExeclNode* node = container->node;
    if(!node->mIsActived) { //第一次Active
        err = node->onActive(mThis_);
        if(err) goto ERROR;
        node->mIsActived = true;
    }
    
    err = node->onExecuting(mThis_, stat);
    if(err) goto ERROR;
    
    switch(stat) {
        case EXECLNODEEXECSTAT_DONE:
        case EXECLNODEEXECSTAT_FASTDONE: {
            bool isHold = false;
            node->onDone(mThis_, isHold);
            if(isHold) {//提交holdNode序列
                mHoldNode = node;
            } else {
                node->~ExeclNode();
            }
            
            mQueue.pop_front();
            break;
        }
        default:
            ;
    }
    
    if(stat == EXECLNODEEXECSTAT_FASTDONE)
        processFrontNode();

    return;

ERROR:
    mThis_->setAllNodesError(err);
}

ExeclQueue::ExeclQueue()
{
    mImpl_ = new ExeclQueueImpl();
    mImpl_->mThis_ = this;
}

ExeclQueue::~ExeclQueue()
{
    delete mImpl_;
}

void ExeclQueue::processExeclNode(void)
{
    MC_ErrorCode err;
    ExeclNodeExecStat stat = EXECLNODEEXECSTAT_BUSY;
    
    if(mImpl_->mHoldNode) {
        if(!mImpl_->mQueue.empty()) { //打断holdNode
            mImpl_->mHoldNode->onAborted(this);
            mImpl_->mHoldNode->~ExeclNode();
            mImpl_->mHoldNode = nullptr;
        } else {
            err = mImpl_->mHoldNode->onExecuting(this, stat);
            if(err) {
                setAllNodesError(err);
                return;
            }
        }
    }
    
    mImpl_->processFrontNode();
    
    return;
}

MC_ErrorCode ExeclQueue::pushAndNewData(
    const std::function<ExeclNode*(void*)>& constructor,
    bool abortFlag)
{
    if(abortFlag)
        setAllNodesAborted();
    
    if(mImpl_->mQueue.push_back()) {
        ExeclNodeContainer* container = mImpl_->mQueue.back();
        container->node = constructor(container->data);
        container->node->mContainer = container;
        return MC_ERRORCODE_GOOD;
    } else {
        return MC_ERRORCODE_QUEUEFULL;
    }
}

ExeclNode* ExeclQueue::front(void) const
{
    ExeclNodeContainer* container = mImpl_->mQueue.front();
    return container? container->node: nullptr;
        
}

ExeclNode* ExeclQueue::back(void) const
{
    ExeclNodeContainer* container = mImpl_->mQueue.back();
    return container? container->node: nullptr;
}

ExeclNode* ExeclQueue::next(ExeclNode* node) const
{
    ExeclNodeContainer* container = node->mContainer;
    ExeclNodeContainer* containerNext = mImpl_->mQueue.next(container);
    
    return containerNext? containerNext->node: nullptr;
}

ExeclNode* ExeclQueue::prev(ExeclNode* node) const
{
    ExeclNodeContainer* container = node->mContainer;
    ExeclNodeContainer* containerPrev = mImpl_->mQueue.prev(container);
    
    return containerPrev? containerPrev->node: nullptr;
}

bool ExeclQueue::busy(void) const
{
    return !(mImpl_->mQueue.empty() && !mImpl_->mHoldNode);
}

size_t ExeclQueue::operationRemains(void) const
{
    return mImpl_->mQueue.used();
}

void ExeclQueue::setAllNodesAborted(void)
{
    if(mImpl_->mHoldNode) {
        mImpl_->mHoldNode->onAborted(this);
        mImpl_->mHoldNode->~ExeclNode();
        mImpl_->mHoldNode = nullptr;
    }
    
    ExeclNodeContainer* container;
    while((container = mImpl_->mQueue.front())) {
        ExeclNode* node = container->node;
        node->onAborted(this);
        node->~ExeclNode();
        mImpl_->mQueue.pop_front();
    }
    
    URANUS_CALL_EVENT(onAllNodesAborted, this);
}

void ExeclQueue::setAllNodesError(MC_ErrorCode errorCodeToSet)
{
    if(mImpl_->mHoldNode) {
        mImpl_->mHoldNode->onError(this, errorCodeToSet);
        mImpl_->mHoldNode->~ExeclNode();
        mImpl_->mHoldNode = nullptr;
    }
    
    ExeclNodeContainer* container;
    while((container = mImpl_->mQueue.front())) {
        ExeclNode* node = container->node;
        node->onError(this, errorCodeToSet);
        node->~ExeclNode();
        mImpl_->mQueue.pop_front();
    }
    
    URANUS_CALL_EVENT(onAllNodesError, this, errorCodeToSet);
}

}
