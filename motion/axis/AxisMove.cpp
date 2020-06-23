/*
 * AxisMove.cpp
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
 
#include "AxisMove.hpp"
#include "FunctionBlock.hpp"
#include "ProfilesPlanner.hpp"
#include "MathUtils.hpp"
#include "Event.hpp"

namespace Uranus {
    
class MoveNode : virtual public AxisExeclNode, public ProfileNode
{
public:
    bool mNeedPlan = true;
    bool mIsHold = false;
    
protected:
    virtual MC_ErrorCode onExecuting(ExeclQueue* queue, ExeclNodeExecStat& stat) override;
    virtual void onDone(ExeclQueue* queue, bool& isHold) override;
    virtual void onPositionOffset(ExeclQueue* queue, double positionOffset) override;
};

class AxisMove::AxisMoveImpl
{
public:
    AxisMove* mThis_;
    ProfilesPlanner mPlanner;

public:
    MC_ErrorCode addMove(
        FunctionBlock* fb, 
        double pos, 
        double vel, 
        double acc, 
        double dec, 
        double endVel, 
        double jerk, 
        MC_ShiftingMode shiftingMode,
        MC_Direction dir, 
        MC_BufferMode bufferMode,
        MC_AxisStatus statusActive,
        MC_AxisStatus statusDone,
        bool isHold, 
        int32_t customId);
};

MC_ErrorCode MoveNode::onExecuting(
    ExeclQueue* queue, ExeclNodeExecStat& stat)
{
    AxisMove* axis = dynamic_cast<AxisMove*>(queue);
    ProfilesPlanner* planner = &axis->mImpl_->mPlanner;
    
    if(mNeedPlan) {
        mNeedPlan = false;
        
        bool ret = planner->plan(
            this, 
            axis->cmdPosition(), 
            axis->cmdVelocity(), 
            axis->cmdAcceleration());
            
        axis->printLog(MC_LOGLEVEL_DEBUG, 
            "MovePos %lf -> %lf, Vel %lf -> %lf, With MaxVel %lf, MaxAcc %lf, MaxDec %lf, Jerk %lf\n", 
            axis->cmdPosition(),
            mEndPos, 
            axis->cmdVelocity(), 
            mEndVel,
            mVel,
            mAcc,
            mDec,
            mJerk);
            
        if(!ret) {
            stat = EXECLNODEEXECSTAT_FASTDONE;
            planner->execute();
            axis->setPosition(
                planner->getEndPosition(), 
                planner->getEndVelocity(),
                0);
            return MC_ERRORCODE_GOOD;
        }
    }
    
    if(planner->execute())
        stat = EXECLNODEEXECSTAT_DONE;
    
    return axis->setPosition(
        planner->getPosition(), 
        planner->getVelocity(), 
        planner->getAcceleration());
}

void MoveNode::onDone(ExeclQueue* queue, bool& isHold)
{
    AxisExeclNode::onDone(queue, isHold);
    isHold = mIsHold;
}

void MoveNode::onPositionOffset(ExeclQueue* queue, double positionOffset)
{
    mStartPos += positionOffset;
    mEndPos += positionOffset;
}

MC_ErrorCode AxisMove::AxisMoveImpl::addMove(
    FunctionBlock* fb, 
    double pos, 
    double vel, 
    double acc, 
    double dec, 
    double endVel, 
    double jerk, 
    MC_ShiftingMode shiftingMode,
    MC_Direction dir, 
    MC_BufferMode bufferMode,
    MC_AxisStatus statusActive, 
    MC_AxisStatus statusDone,
    bool isHold, 
    int32_t customId)
{
    MoveNode* node, * nodePrev;
    MC_ErrorCode err;
    
    //速度参数检测
    if((vel < 0 && !std::isnan(pos)) || !std::isfinite(vel))
        return MC_ERRORCODE_VELILLEGAL;
    
    //加速度参数检测
    if(acc <= 0 || !std::isfinite(acc) || dec <= 0 || !std::isfinite(dec))
        return MC_ERRORCODE_ACCILLEGAL;
    
    //位置参数检测
    if(std::isinf(pos))
        return MC_ERRORCODE_POSILLEGAL;
    
    //起始位置处理
    double startPos, startVel, startAcc;
    if(shiftingMode == MC_SHIFTINGMODE_ADDITIVE || 
        bufferMode != MC_BUFFERMODE_ABORTING) { //使用最后一个功能块终点位置
        if(!mThis_->operationRemains()) goto USE_CURRENT;
        nodePrev = dynamic_cast<MoveNode*>(mThis_->back());
        if(!nodePrev) return MC_ERRORCODE_FAILEDTOBUFFER;
        startPos = nodePrev->mEndPos;
        startVel = nodePrev->mEndVel;
        startAcc = nodePrev->mEndAcc;
    } else { //使用当前位置
    USE_CURRENT:
        startPos = mThis_->cmdPosition();
        startVel = mThis_->cmdVelocity();
        startAcc = mThis_->cmdAcceleration();
    }
    
    //特殊位置处理
    if(std::isnan(pos)) { //不清楚位置
        pos = ProfilePlanner::calculateDist(startVel, vel, acc, dec);
        pos += startPos;
    } else {
        switch(shiftingMode) {
            case MC_SHIFTINGMODE_ABSOLUTE: { //绝对位置的模量与零点偏移转换
                pos = mThis_->userPosToSys(startPos, pos, dir);
                break;
            }
            
            case MC_SHIFTINGMODE_RELATIVE: //相对位置
            case MC_SHIFTINGMODE_ADDITIVE: //增量位置
                pos += startPos;
                break;
            
            default:
                return MC_ERRORCODE_SHIFTINGMODEILLEGAL;
        }
    }
        
    //添加队列
    err = mThis_->pushAndNewData(
        [&](void* baseNode) -> AxisExeclNode* {
            //构造数据
            node = (MoveNode*)baseNode;
            new (node) MoveNode();
            node->mStartPos = startPos;
            node->mStartVel = startVel;
            node->mStartAcc = startAcc;
            node->mEndPos = pos;
            node->mEndVel = endVel;
            node->mEndAcc = 0;
            node->mVel = vel;
            node->mAcc = acc;
            node->mDec = dec;
            node->mJerk = jerk;
            
            node->mIsHold = isHold;
            return node;
        }, 
        (bufferMode == MC_BUFFERMODE_ABORTING), 
        fb, 
        statusActive, 
        statusDone, 
        customId);
        
    return err;
}

AxisMove::AxisMove()
{
    mImpl_ = new AxisMoveImpl();
    mImpl_->mThis_ = this;
    
    URANUS_ADD_HANDLER(onPowerStatusChanged, onPowerStatusChangedHandler);
    URANUS_ADD_HANDLER(onPositionOffset, onPositionOffsetHandler);
    URANUS_ADD_HANDLER(onAllNodesAborted, onAllNodesAbortedHandler);
    URANUS_ADD_HANDLER(onAllNodesError, onAllNodesErrorHandler);
}

AxisMove::~AxisMove()
{
    delete mImpl_;
}

MC_ErrorCode AxisMove::addMovePos(
    FunctionBlock* fb, 
    double pos, 
    double vel, 
    double acc, 
    double dec, 
    double jerk, 
    MC_ShiftingMode shiftingMode,
    MC_Direction dir, 
    MC_BufferMode bufferMode,
    int32_t customId)
{
    if(!vel)
        return MC_ERRORCODE_VELILLEGAL;
        
    return mImpl_->addMove(
        fb, 
        pos, 
        vel, 
        acc, 
        dec, 
        0, 
        jerk, 
        shiftingMode, 
        dir, 
        bufferMode,
        MC_AXISSTATUS_DISCRETEMOTION,
        MC_AXISSTATUS_STANDSTILL,
        false, 
        customId);
}

MC_ErrorCode AxisMove::addMovePosCont(
    FunctionBlock* fb, 
    double pos, 
    double vel, 
    double acc, 
    double dec, 
    double endVel, 
    double jerk, 
    MC_ShiftingMode shiftingMode,
    MC_Direction dir, 
    MC_BufferMode bufferMode,
    int32_t customId)
{
    if(!endVel || !vel)
        return MC_ERRORCODE_VELILLEGAL;
    
    return mImpl_->addMove(
        fb, 
        pos, 
        vel, 
        acc, 
        dec, 
        endVel, 
        jerk, 
        shiftingMode, 
        dir, 
        bufferMode,
        MC_AXISSTATUS_CONTINUOUSMOTION,
        MC_AXISSTATUS_CONTINUOUSMOTION,
        true, 
        customId);
}
    
MC_ErrorCode AxisMove::addMoveVel(
    FunctionBlock* fb, 
    double vel, 
    double acc, 
    double dec, 
    double jerk, 
    MC_BufferMode bufferMode,
    int32_t customId)
{
    if(!vel)
        return MC_ERRORCODE_VELILLEGAL;
    
    return mImpl_->addMove(
        fb, 
        NAN, 
        vel, 
        acc, 
        dec, 
        vel, 
        jerk, 
        MC_SHIFTINGMODE_ABSOLUTE, 
        MC_DIRECTION_CURRENT, 
        bufferMode,
        MC_AXISSTATUS_CONTINUOUSMOTION,
        MC_AXISSTATUS_CONTINUOUSMOTION,
        true, 
        customId);
}

MC_ErrorCode AxisMove::addHalt(
    FunctionBlock* fb, 
    double dec, 
    double jerk, 
    MC_BufferMode bufferMode,
    int32_t customId)
{
    return mImpl_->addMove(
        fb, 
        NAN, 
        __EPSILON, 
        dec, 
        dec, 
        0, 
        jerk, 
        MC_SHIFTINGMODE_ABSOLUTE, 
        MC_DIRECTION_CURRENT, 
        bufferMode,
        MC_AXISSTATUS_DISCRETEMOTION,
        MC_AXISSTATUS_STANDSTILL,
        false, 
        customId);
}
    
MC_ErrorCode AxisMove::addStop(
    FunctionBlock* fb, 
    double dec, 
    double jerk,
    int32_t customId)
{
    return mImpl_->addMove(
        fb, 
        NAN, 
        __EPSILON, 
        dec, 
        dec, 
        0, 
        jerk, 
        MC_SHIFTINGMODE_ABSOLUTE, 
        MC_DIRECTION_CURRENT, 
        MC_BUFFERMODE_ABORTING,
        MC_AXISSTATUS_STOPPING,
        MC_AXISSTATUS_STOPPING,
        false, 
        customId);
}
       
void AxisMove::cancelStopLater(void)
{
    if(status() != MC_AXISSTATUS_STOPPING)
        return;
        
    if(!operationRemains()) {
        setStatus(MC_AXISSTATUS_STANDSTILL);
    } else {
        AxisExeclNode* node = dynamic_cast<AxisExeclNode*>(front());
        if(!node)
            return;
            
        if(node->mStatusDone == MC_AXISSTATUS_STOPPING)
            node->mStatusDone = MC_AXISSTATUS_STANDSTILL;
    }
}

void AxisMove::onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus)
{
    AxisMove* this__ = dynamic_cast<AxisMove*>(this_);
    if(powerStatus)
        this__->mImpl_->mPlanner.setFrequency(this__->frequency());
}

void AxisMove::onPositionOffsetHandler(AxisBase* this_, double positionOffset)
{
    AxisMove* this__ = dynamic_cast<AxisMove*>(this_);
    this__->mImpl_->mPlanner.setPositionOffset(positionOffset);
}

void AxisMove::onAllNodesAbortedHandler(ExeclQueue* this_)
{
    //AxisMove* this__ = dynamic_cast<AxisMove*>(this_);
}

void AxisMove::onAllNodesErrorHandler(
    ExeclQueue* this_, MC_ErrorCode errorCodeToSet)
{
    //AxisMove* this__ = dynamic_cast<AxisMove*>(this_);
}

}
