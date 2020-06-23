/*
 * Servo.cpp
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
 
#include "Servo.hpp"
#include "Scheduler.hpp"

namespace Uranus {

class Servo::ServoImpl
{
public:
    int32_t mSubmitPos = 0;
    int32_t mPos = 0;
    double mVel = 0;
    double mAcc = 0;
};

Servo::Servo()
{
    mImpl_ = new ServoImpl();
}

Servo::~Servo()
{
    delete mImpl_;
}

MC_ServoErrorCode Servo::setPower(bool powerStatus, bool& isDone)
{
    isDone = true;
    return 0;
}

MC_ServoErrorCode Servo::setPos(int32_t pos)
{
    mImpl_->mSubmitPos = pos;
    return 0;
}

MC_ServoErrorCode Servo::setVel(int32_t vel)
{
    return 0xFFFFFFFF;
}

MC_ServoErrorCode Servo::setTorque(double torque)
{
    return 0xFFFFFFFF;
}

int32_t Servo::pos(void)
{
    return mImpl_->mPos;
}

int32_t Servo::vel(void)
{
    return mImpl_->mVel;
}

int32_t Servo::acc(void)
{
    return mImpl_->mAcc;
}

double Servo::torque(void)
{
    return 0;
}

bool Servo::readVal(int index, double& value)
{
    return false;
}

bool Servo::writeVal(int index, double value)
{
    return false;
}

MC_ServoErrorCode Servo::resetError(bool& isDone)
{
    isDone = true;
    return 0;
}

void Servo::runCycle(double freq)
{
    int32_t posDiff = mImpl_->mSubmitPos - mImpl_->mPos;
    double curVel = posDiff * freq;
    mImpl_->mAcc = (curVel - mImpl_->mVel) * freq;
    mImpl_->mVel = curVel;
    mImpl_->mPos = mImpl_->mSubmitPos;
}

void Servo::emergStop(void)
{
    mImpl_->mPos = mImpl_->mSubmitPos;
    mImpl_->mVel = mImpl_->mAcc = 0;
}

}
