/*
 * Scheduler.cpp
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

#include "Scheduler.hpp"
#include "Axis.hpp"

namespace Uranus {

class Scheduler::SchedulerImpl
{
public:
    Axis mAxisHead;
    double mFreq = 1000.0;
    uint32_t mTick = 0;
};

Scheduler::Scheduler()
{
    mImpl_ = new SchedulerImpl();
}

Scheduler::~Scheduler()
{
    delete mImpl_;
}

void Scheduler::runCycle(void)
{
    Axis* axis = axisListFirst();
    while(axis) {
        axis->runCycle();
        axis = axisListNext(axis);
    }
    
    ++mImpl_->mTick;
}

MC_ErrorCode Scheduler::setFrequency(double frequency)
{
    if(frequency <= 0)
        return MC_ERRORCODE_FREQUENCYILLEGAL;
    
    if(axisListFirst())
        return MC_ERRORCODE_AXISBUSY;
        
    mImpl_->mFreq = frequency;
    
    return MC_ERRORCODE_GOOD;
}

double Scheduler::frequency(void) const
{
    return mImpl_->mFreq;
}

uint32_t Scheduler::tick(void) const
{
    return mImpl_->mTick;
}

Axis* Scheduler::newAxis(int32_t axisId, Servo* servo)
{
    if(axis(axisId))
        return nullptr;
        
    Axis* newAxis = new Axis();
    if(!servo)
        servo = new Servo();
        
    newAxis->setServo(servo);
    newAxis->mSched = this;
    newAxis->mAxisId = axisId;
    newAxis->insertBack(&mImpl_->mAxisHead);
    
    return newAxis;
}
    
Axis* Scheduler::axis(int32_t axisId) const
{
    Axis* axis = axisListFirst();
    while(axis) {
        if(axis->mAxisId == axisId)
            return axis;
        axis = axisListNext(axis);
    }
    
    return nullptr;
}

MC_ErrorCode Scheduler::setAxisConfig(
    Axis* axis, const AxisConfig& config)
{
    MC_ErrorCode err;
    err = axis->setControlInfo(config.mControlInfo);
    if(err) return err;
    err = axis->setMetricInfo(config.mMetricInfo);
    if(err) return err;
    err = axis->setMotionLimitInfo(config.mMotionLimitInfo);
    if(err) return err;
    err = axis->setRangeLimitInfo(config.mRangeLimitInfo);
    if(err) return err;
    err = axis->setHomingInfo(config.mHomingInfo);
    return err;
}

MC_ErrorCode Scheduler::setAxisHomePosition(Axis* axis, double homePos)
{
    return axis->setHomePosition(homePos);
}

Axis* Scheduler::axisListFirst(void) const
{
    return dynamic_cast<Axis*>(mImpl_->mAxisHead.LinkNode::next());
}
    
Axis* Scheduler::axisListNext(const Axis* one) const
{
    return dynamic_cast<Axis*>(one->LinkNode::next());
}

void Scheduler::release(void)
{
    LinkNode* node;
    while((node = axisListFirst())) {
        node->takeOut();
        delete node;
    }
}

}
