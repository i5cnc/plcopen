/*
 * AxisHoming.cpp
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

#include "AxisHoming.hpp"
#include "FunctionBlock.hpp"
#include "ProfilePlanner.hpp"
#include "MathUtils.hpp"
#include "Event.hpp"

namespace Uranus {

typedef enum {
    MC_HOMINGSTEP_INIT              = 0,
    MC_HOMINGSTEP_SEARCHSIG         = 1,
    MC_HOMINGSTEP_REGRESSIONSIG     = 2,
    MC_HOMINGSTEP_TOSIG             = 3,
}MC_HomingStep;
    
struct AxisHomingInfoEx : public AxisHomingInfo
{
    bool mHomingSigVal = false; //回零信号比对值
};

class AxisHoming::AxisHomingImpl
{
public:
    AxisHomingInfoEx mHomingInfo;
    ProfilePlanner mPlanner;
};

class HomingNode : virtual public AxisExeclNode
{
public:
    double mPos = 0;
    double mFinalPos = 0;
    MC_HomingStep mHomingStep = MC_HOMINGSTEP_INIT;
    
protected:
    virtual MC_ErrorCode onExecuting(ExeclQueue* queue, ExeclNodeExecStat& stat) override;
    virtual void onPositionOffset(ExeclQueue* queue, double offset) override;
};

MC_ErrorCode HomingNode::onExecuting(
    ExeclQueue* queue, ExeclNodeExecStat& stat)
{
    AxisHoming* axis = dynamic_cast<AxisHoming*>(queue);
    ProfilePlanner* planner = &axis->mImpl_->mPlanner;
    AxisHomingInfoEx* homingInfo = &axis->mImpl_->mHomingInfo;
    MC_ErrorCode err;
    
    switch(mHomingStep) {
        case MC_HOMINGSTEP_INIT:
            if(!homingInfo->mHomingSig) { //当前位置作为零点
                goto HOMINGSTEP_TOSIG;
            } else { //启动回零流程
                axis->printLog(MC_LOGLEVEL_INFO, 
                    "homing start, vel %lf, sig %p, bitoffset %d, trig %d\n",
                    homingInfo->mHomingVelSearch,
                    homingInfo->mHomingSig,
                    homingInfo->mHomingSigBitOffset,
                    homingInfo->mHomingSigVal);
                    
                if(!homingInfo->mHomingVelSearch || !homingInfo->mHomingVelRegression)
                    return MC_ERRORCODE_HOMINGVELILLEGAL;
                    
                if(!homingInfo->mHomingAcc)
                    return MC_ERRORCODE_HOMINGACCILLEGAL;
                    
                double endPos = ProfilePlanner::calculateDist(
                    axis->cmdVelocity(),
                    homingInfo->mHomingVelSearch,
                    homingInfo->mHomingAcc,
                    homingInfo->mHomingAcc);
                    
                planner->plan(
                    axis->cmdPosition(), 
                    axis->cmdPosition() + endPos,
                    axis->cmdVelocity(),
                    homingInfo->mHomingVelSearch,
                    homingInfo->mHomingVelSearch,
                    homingInfo->mHomingAcc,
                    homingInfo->mHomingAcc);
                    
                mHomingStep = MC_HOMINGSTEP_SEARCHSIG;
            }
            break;
            
        case MC_HOMINGSTEP_SEARCHSIG:
            if((((*homingInfo->mHomingSig) >> homingInfo->mHomingSigBitOffset) & 0x1) == 
                homingInfo->mHomingSigVal) {
                
                double endPos = ProfilePlanner::calculateDist(
                    axis->cmdVelocity(),
                    homingInfo->mHomingVelRegression,
                    homingInfo->mHomingAcc,
                    homingInfo->mHomingAcc);
                    
                planner->plan(
                    axis->cmdPosition(), 
                    axis->cmdPosition() + endPos,
                    axis->cmdVelocity(),
                    homingInfo->mHomingVelRegression,
                    homingInfo->mHomingVelRegression,
                    homingInfo->mHomingAcc,
                    homingInfo->mHomingAcc);
                    
                mHomingStep = MC_HOMINGSTEP_REGRESSIONSIG;
                
                axis->printLog(MC_LOGLEVEL_INFO, 
                    "homing regressing, vel %lf\n", 
                    homingInfo->mHomingVelRegression);
            }
              
            break;
            
        case MC_HOMINGSTEP_REGRESSIONSIG:
            if((((*homingInfo->mHomingSig) >> homingInfo->mHomingSigBitOffset) & 0x1) != 
                homingInfo->mHomingSigVal) {
HOMINGSTEP_TOSIG:
                mFinalPos = axis->actPosition();
                
                planner->plan(
                    axis->cmdPosition(), 
                    axis->cmdPosition() + 
                    ProfilePlanner::calculateDist(
                        axis->cmdVelocity(), 
                        __EPSILON, 
                        homingInfo->mHomingAcc, 
                        homingInfo->mHomingAcc),
                    axis->cmdVelocity(),
                    homingInfo->mHomingVelRegression,
                    0.0,
                    homingInfo->mHomingAcc,
                    homingInfo->mHomingAcc);
                    
                mHomingStep = MC_HOMINGSTEP_TOSIG;
            }
            
            break;
            
        case MC_HOMINGSTEP_TOSIG:
            if(planner->execute())
                stat = EXECLNODEEXECSTAT_DONE;
            
            err = axis->setPosition(
                planner->getPosition(), 
                planner->getVelocity(), 
                planner->getAcceleration());
                
            if(stat == EXECLNODEEXECSTAT_DONE && err == MC_ERRORCODE_GOOD) {
                err = axis->setHomePosition(mPos - mFinalPos);
                axis->printLog(MC_LOGLEVEL_INFO, 
                    "homing complete, new pos %lf\n",
                    axis->homePosition());
            }
                  
            return err;
    }
    
    planner->execute();
    
    err = axis->setPosition(
        planner->getPosition(), 
        planner->getVelocity(), 
        planner->getAcceleration());
          
    return err;
}

void HomingNode::onPositionOffset(ExeclQueue* queue, double offset)
{
    
}

AxisHoming::AxisHoming()
{
    mImpl_ = new AxisHomingImpl();
    
    URANUS_ADD_HANDLER(onPowerStatusChanged, onPowerStatusChangedHandler);
    URANUS_ADD_HANDLER(onPositionOffset, onPositionOffsetHandler);
}

AxisHoming::~AxisHoming()
{
    delete mImpl_;
}

MC_ErrorCode AxisHoming::setHomingInfo(const AxisHomingInfo& info)
{
    if(info.mHomingMode != MC_HOMINGMODE_DIRECT) {
        if(!info.mHomingVelSearch || !info.mHomingVelRegression)
            return MC_ERRORCODE_HOMINGVELILLEGAL;
            
        if(!info.mHomingAcc)
            return MC_ERRORCODE_HOMINGACCILLEGAL;
            
        if(!info.mHomingSig)
            return MC_ERRORCODE_HOMINGSIGILLEGAL;
            
        if(info.mHomingSigBitOffset < 0 || info.mHomingSigBitOffset > 7)
            return MC_ERRORCODE_HOMINGSIGILLEGAL;
    }
    
    switch(info.mHomingMode) {
        case MC_HOMINGMODE_DIRECT:
            mImpl_->mHomingInfo.mHomingSig = nullptr;
            return MC_ERRORCODE_GOOD;
            
        case MC_HOMINGMODE_MODE5:
            mImpl_->mHomingInfo.mHomingSigVal = true;
        case MC_HOMINGMODE_MODE6:
            mImpl_->mHomingInfo.mHomingVelSearch = 
                -fabs(info.mHomingVelSearch);
                
            mImpl_->mHomingInfo.mHomingVelRegression = 
                fabs(info.mHomingVelRegression);
            break;
            
        case MC_HOMINGMODE_MODE7:
            mImpl_->mHomingInfo.mHomingSigVal = true;
        case MC_HOMINGMODE_MODE8:
            mImpl_->mHomingInfo.mHomingVelSearch = 
                fabs(info.mHomingVelSearch);
                
            mImpl_->mHomingInfo.mHomingVelRegression = 
                -fabs(info.mHomingVelRegression);
            break;
            
        default:
            return MC_ERRORCODE_HOMINGMODEILLEGAL;
    }
    
    mImpl_->mHomingInfo.mHomingSig = info.mHomingSig;
    mImpl_->mHomingInfo.mHomingMode = info.mHomingMode;
    
    mImpl_->mHomingInfo.mHomingAcc = fabs(info.mHomingAcc);
    mImpl_->mHomingInfo.mHomingJerk = fabs(info.mHomingJerk);
    
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode AxisHoming::addHoming(
    FunctionBlock* fb, 
    double pos, 
    MC_BufferMode bufferMode, 
    int32_t customId)
{
    if(!std::isfinite(pos))
        return MC_ERRORCODE_POSILLEGAL;
        
    HomingNode* node;
    MC_ErrorCode err = pushAndNewData(
        [&node, pos](void* baseNode) -> AxisExeclNode* {
        node = (HomingNode*)baseNode;
        new (node) HomingNode();
        node->mPos = pos;
        return node;
    }, 
    (bufferMode == MC_BUFFERMODE_ABORTING), 
    fb, 
    MC_AXISSTATUS_HOMING, 
    MC_AXISSTATUS_STANDSTILL, 
    customId);
    
    if(err) return err;
    
    return MC_ERRORCODE_GOOD;
}

void AxisHoming::onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus)
{
    AxisHoming* this__ = dynamic_cast<AxisHoming*>(this_);
    if(powerStatus)
        this__->mImpl_->mPlanner.setFrequency(this__->frequency());
}

void AxisHoming::onPositionOffsetHandler(AxisBase* this_, double positionOffset)
{
    AxisHoming* this__ = dynamic_cast<AxisHoming*>(this_);
    this__->mImpl_->mPlanner.setPositionOffset(positionOffset);
}

}
