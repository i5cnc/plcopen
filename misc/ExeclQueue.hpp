/*
 * ExeclQueue.hpp
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

#ifndef _URANUS_EXECLQUEUE_HPP_
#define _URANUS_EXECLQUEUE_HPP_

#include "Global.hpp"
#include "Event.hpp"
#include <functional>

namespace Uranus {
    
typedef enum {
    EXECLNODEEXECSTAT_BUSY      = 0,
    EXECLNODEEXECSTAT_DONE      = 1,
    EXECLNODEEXECSTAT_FASTDONE  = 2,
}ExeclNodeExecStat;
    
struct ExeclNodeContainer;
class ExeclQueue;
class ExeclNode
{
public:
    ExeclNode();
    virtual ~ExeclNode();
    
protected:
    virtual MC_ErrorCode onActive(ExeclQueue* queue) = 0;
    virtual MC_ErrorCode onExecuting(ExeclQueue* queue, ExeclNodeExecStat& stat) = 0;
    virtual void onAborted(ExeclQueue* queue) = 0;
    virtual void onDone(ExeclQueue* queue, bool& isHold) = 0;
    virtual void onError(ExeclQueue* queue, MC_ErrorCode errorCode) = 0;
    
private:
    bool mIsActived = false;
    ExeclNodeContainer* mContainer = nullptr;
    friend class ExeclQueue;
};

class ExeclQueue
{
public:
    ExeclQueue();
    virtual ~ExeclQueue();
    
    void processExeclNode(void);
    MC_ErrorCode pushAndNewData(
        const std::function<ExeclNode*(void*)>& constructor,
        bool abortFlag);
        
    ExeclNode* front(void) const;
    ExeclNode* back(void) const;
    ExeclNode* next(ExeclNode* node) const;
    ExeclNode* prev(ExeclNode* node) const;
    bool busy(void) const;
    size_t operationRemains(void) const;
    void setAllNodesAborted(void);
    void setAllNodesError(MC_ErrorCode errorCodeToSet);
    
protected:
    URANUS_DEFINE_EVENT(onAllNodesAborted, ExeclQueue*);
    URANUS_DEFINE_EVENT(onAllNodesError, ExeclQueue*, MC_ErrorCode);
    
private:
    class ExeclQueueImpl;
    ExeclQueueImpl* mImpl_;
};

}

#endif /** _URANUS_EXECLQUEUE_HPP_ **/
