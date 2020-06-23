/*
 * AxisBase.cpp
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

#include "AxisBase.hpp"
#include "Servo.hpp"
#include "MathUtils.hpp"
#include "Event.hpp"

#include <cstring>

namespace Uranus {

#define URANUS_AXISNAMESIZE 64

class AxisBase::AxisBaseImpl
{
public:
    AxisBase* mThis_;
    
    Servo* mServo = nullptr;
    
    char mAxisName[URANUS_AXISNAMESIZE] = "Axis";
    AxisMetricInfo mMetric;
    AxisRangeLimitInfo mRangeLimit;
    AxisMotionLimitInfo mMotionLimit;
    AxisControlInfo mControl;
    double mHomePos = 0;

    MC_ErrorCode mErrorCode = MC_ERRORCODE_GOOD;
    MC_ServoErrorCode mDevErrorCode = 0;
    bool mNeedReset = false;
    
    bool mPowerStatus = false;
    bool mPowerStatusValid = false;

    double mSubmitCmdPos = 0;
    double mCmdPos = 0;
    double mCmdVel = 0;
    double mCmdAcc = 0;
    
    double mCalVel = 0;
    
    bool mEnablePositive = false;
    bool mEnableNegative = false;
    
    double mEncoderOverflowOffset = 0;
    
public:
    void processPositionLoop(void);
    void servoStatusMaintains(void);
    void updateCmdPosToDev(void);
    
    int32_t toDevRaw(double x) const;
    double toSystemLogic(double x) const;

    double moduloToLinear(double basePos, double targetPos, MC_Direction dir) const;
    double linearToModulo(double pos) const;
    double packHomeOffset(double pos) const;
    double stripHomeOffset(double basePos, double pos) const;
};

void AxisBase::AxisBaseImpl::processPositionLoop(void)
{
    if(mThis_->errorCode())
        return;
    
    if(!mThis_->powerStatus())
        return;
    
    double calVel = (mSubmitCmdPos - mCmdPos) * mThis_->frequency();
    
    if(calVel > 0 && !mEnablePositive) {
        mThis_->emergStop(MC_ERRORCODE_FORBIDDENPPOSMOVE);
        return;
    } else if(calVel < 0 && !mEnableNegative) {
        mThis_->emergStop(MC_ERRORCODE_FORBIDDENNPOSMOVE);
        return;
    }
    
    //printf("Axis error %lf %lf %lf %lf\n", mSubmitCmdPos, mCmdPos, calVel, mMotionLimit.mVelLimit);
    if(__isgt(fabs(calVel), mMotionLimit.mVelLimit)) {
        mThis_->emergStop(MC_ERRORCODE_CMDVELOVERLIMIT);
        return;
    }
    /*
    double calAcc = (calVel - mCalVel) * mThis_->frequency();
    if(__isgt(fabs(calAcc), mMotionLimit.mAccLimit)) {
        mThis_->emergStop(MC_ERRORCODE_CMDACCOVERLIMIT);
        return;
    }*/
    
    if(mRangeLimit.mSwLimitPositive && 
        mRangeLimit.mLimitPositive < mThis_->sysPosToUser(mSubmitCmdPos) &&
        calVel > 0) {
        mThis_->emergStop(MC_ERRORCODE_CMDPPOSOVERLIMIT);
        return;
    } 
    
    if(mRangeLimit.mSwLimitNegative && 
        mRangeLimit.mLimitNegative > mThis_->sysPosToUser(mSubmitCmdPos) &&
        calVel < 0) {
        mThis_->emergStop(MC_ERRORCODE_CMDNPOSOVERLIMIT);
        return;
    } 
    
    mCmdPos = mSubmitCmdPos;
    mCalVel = calVel;
    
    double _2147 = fabs(toSystemLogic(2147483648.0));
    double _4294 = fabs(toSystemLogic(4294967296.0));
    if(mCmdPos >= _2147) {
        mEncoderOverflowOffset = -_4294;
    } else if(mCmdPos < -_2147) {
        mEncoderOverflowOffset = _4294;
    } else {
        mEncoderOverflowOffset = 0;
    }
    
    if(mEncoderOverflowOffset) {
        URANUS_CALL_EVENT(mThis_->onPositionOffset, mThis_, mEncoderOverflowOffset);
        mCmdPos += mEncoderOverflowOffset;
    }
    
    double curDevPos = toSystemLogic(mServo->pos());
    double posDiff = fabs(mCmdPos - curDevPos);
    
    if(mControl.mControlMode != MC_CONTROLMODE_VELOPENLOOP) {
        if(__isgt(posDiff, mMotionLimit.mPosLagLimit)) {
            posDiff -= fabs(toSystemLogic(4294967296.0));
            posDiff = fabs(posDiff);
            if(__isgt(posDiff, mMotionLimit.mPosLagLimit)) {
                mThis_->emergStop(MC_ERRORCODE_POSLAGOVERLIMIT);
                return;
            }
        }
    }
    
    updateCmdPosToDev();
}

void AxisBase::AxisBaseImpl::servoStatusMaintains(void)
{
    //处理错误重置
    if(mNeedReset) {
        if(mThis_->errorCode()) { //存在错误尝试恢复
            bool isDone = false;
            //驱动器恢复
            MC_ServoErrorCode devErrorCode = mServo->resetError(isDone);
            if(devErrorCode) { //驱动器恢复过程中出错，恢复失败
                mDevErrorCode = devErrorCode;
                mNeedReset = false;
            } else if(isDone) { //恢复成功
                mDevErrorCode = 0;
                mErrorCode = MC_ERRORCODE_GOOD;
                mNeedReset = false;
            }
        } else { //不存在错误则直接恢复
            mNeedReset = false;
        }
    }
    
    if(!mThis_->errorCode() && !mPowerStatusValid) {
        //处理驱动器使能
        if(mPowerStatus) { //开
            //同步实际参数到指令参数
            mCmdVel = mCmdAcc = 0;
            mCmdPos = toSystemLogic(mServo->pos());
            mSubmitCmdPos = mCmdPos;
            updateCmdPosToDev();
        }
        bool isDone = false;
        mDevErrorCode = mServo->setPower(mPowerStatus, isDone);
        if(mDevErrorCode) { //使能失败
            mThis_->emergStop(MC_ERRORCODE_AXISHARDWARE);
        } else if(isDone) { //使能成功
            if(mPowerStatus) {
                mThis_->printLog(MC_LOGLEVEL_INFO, "Power on\n");
            } else {
                mThis_->printLog(MC_LOGLEVEL_INFO, "Power off\n");
            }
            mPowerStatusValid = true;
            URANUS_CALL_EVENT(mThis_->onPowerStatusChanged, mThis_, mPowerStatus);
        }
    } else if(!mPowerStatus && mPowerStatusValid) {
        //处理非使能时的指令与实际同步
        mCmdPos = toSystemLogic(mServo->pos());
        mCmdVel = toSystemLogic(mServo->vel());
        mCmdAcc = toSystemLogic(mServo->acc());
    }
}

void AxisBase::AxisBaseImpl::updateCmdPosToDev(void)
{
    double mCmdPosWithFF = mCmdPos + mControl.mFF * 0.01 * mCmdVel;
    
    switch(mControl.mControlMode) {
        case MC_CONTROLMODE_POSOPENLOOP: { //位置控制模式
            int32_t rawPos = toDevRaw(mCmdPosWithFF);
            mDevErrorCode = mServo->setPos(rawPos);
            break;
        }
            
        case MC_CONTROLMODE_VELCLOSELOOP: { //速度闭环控制模式 
            int32_t rawVel = toDevRaw(mCmdPosWithFF) - mServo->pos();
            rawVel *= mControl.mPKp;
            mDevErrorCode = mServo->setVel(rawVel);
            break;
        }
        
        case MC_CONTROLMODE_VELOPENLOOP: { //速度开环控制模式
            int32_t rawVel = toDevRaw(mCmdVel);
            mDevErrorCode = mServo->setVel(rawVel);
            break;
        }
    }
    
    if(mDevErrorCode)
        mThis_->emergStop(MC_ERRORCODE_AXISHARDWARE);
}

inline int32_t AxisBase::AxisBaseImpl::toDevRaw(double x) const
{
    union {
        int32_t _32;
        int64_t _64;
    }raw;
    raw._64 = (int64_t)(x * mMetric.mDevUnitRatio);
    return raw._32;
}

inline double AxisBase::AxisBaseImpl::toSystemLogic(double x) const
{
    return x / mMetric.mDevUnitRatio;
}

double AxisBase::AxisBaseImpl::moduloToLinear(
    double basePos, double targetPos, MC_Direction dir) const
{
    if(!mMetric.mModulo)
        return targetPos;
        
    double targetPosMod = linearToModulo(targetPos);
    double basePosAlign = basePos - linearToModulo(basePos);
    double targetPosCurrent = basePosAlign + targetPosMod;
    
    switch(dir) {
        case MC_DIRECTION_POSITIVE:
            if(targetPosCurrent > basePos)
                return targetPosCurrent;
            else
                return targetPosCurrent + mMetric.mModulo;
                
        case MC_DIRECTION_NEGATIVE:
            if(targetPosCurrent < basePos)
                return targetPosCurrent;
            else
                return targetPosCurrent - mMetric.mModulo;
                
        case MC_DIRECTION_SHORTESTWAY:
        {
            double targetPosSide;
            if(targetPosCurrent > basePos)
                targetPosSide = targetPosCurrent - mMetric.mModulo;
            else
                targetPosSide = targetPosCurrent + mMetric.mModulo;
            
            double distCurrent = fabs(targetPosCurrent - basePos);
            double distSide = fabs(targetPosSide - basePos);
            if(distCurrent < distSide)
                return targetPosCurrent;
            else
                return targetPosSide;
        }
            
        case MC_DIRECTION_CURRENT:
        default:
            return targetPosCurrent;
    }
}

double AxisBase::AxisBaseImpl::linearToModulo(double pos) const
{
    if(!mMetric.mModulo)
        return pos;
    
    double posMod = fmod(pos, mMetric.mModulo);
    if(posMod < 0)
        posMod += mMetric.mModulo;
        
    return posMod;
}

double AxisBase::AxisBaseImpl::packHomeOffset(double pos) const
{
    double posWithHomeOffset = pos + mHomePos;
    double _2147 = fabs(toSystemLogic(2147483648.0));
    double _4294 = fabs(toSystemLogic(4294967296.0));
    if(posWithHomeOffset >= _2147) {
        posWithHomeOffset -= _4294;
    } else if(posWithHomeOffset < -_2147) {
        posWithHomeOffset += _4294;
    }
    return posWithHomeOffset;
}

double AxisBase::AxisBaseImpl::stripHomeOffset(double basePos, double pos) const
{ 
    return pos - packHomeOffset(basePos) + basePos;
}

/////////////////////////////////////////////////////////////

AxisBase::AxisBase()
{
    mImpl_ = new AxisBaseImpl();
    mImpl_->mThis_ = this;
}

AxisBase::~AxisBase()
{
    if(mImpl_->mServo)
        delete mImpl_->mServo;
    delete mImpl_;
}

void AxisBase::runCycle(void)
{
    mImpl_->servoStatusMaintains();
    mImpl_->processPositionLoop();
    mImpl_->mServo->runCycle(frequency());
}

void AxisBase::setServo(Servo* servo)
{
    mImpl_->mServo = servo;
}

void AxisBase::setAxisName(const char* name)
{
    strncpy(mImpl_->mAxisName, name, URANUS_AXISNAMESIZE);
    mImpl_->mAxisName[URANUS_AXISNAMESIZE - 1] = '\0';
}

MC_ErrorCode AxisBase::setMetricInfo(const AxisMetricInfo& info)
{
    if(powerStatus())
        return MC_ERRORCODE_AXISPOWERON;
        
    if(errorCode())
        return errorCode();
        
    if(fabs(info.mDevUnitRatio) < 1.0 || 
        fabs(info.mDevUnitRatio) > 1048576.0 ||
        !std::isfinite(info.mDevUnitRatio))
        return MC_ERRORCODE_CFGUNITRATIOOUTOFRANGE;
        
    if(info.mModulo < 0 || !std::isfinite(info.mModulo))
        return MC_ERRORCODE_CFGMODULOILLEGAL;
        
    mImpl_->mMetric = info;
    
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode AxisBase::setRangeLimitInfo(const AxisRangeLimitInfo& info)
{
    mImpl_->mRangeLimit = info;
    
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode AxisBase::setMotionLimitInfo(const AxisMotionLimitInfo& info)
{
    if(powerStatus())
        return MC_ERRORCODE_AXISPOWERON;
        
    if(info.mVelLimit <= 0 || !std::isfinite(info.mVelLimit))
        return MC_ERRORCODE_CFGVELLIMITILLEGAL;
        
    if(info.mAccLimit <= 0 || !std::isfinite(info.mAccLimit))
        return MC_ERRORCODE_CFGACCLIMITILLEGAL;
        
    if(info.mPosLagLimit <= 0 || !std::isfinite(info.mPosLagLimit))
        return MC_ERRORCODE_CFGPOSLAGILLEGAL;
        
    mImpl_->mMotionLimit = info;
    
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode AxisBase::setControlInfo(const AxisControlInfo& info)
{
    if(powerStatus())
        return MC_ERRORCODE_AXISPOWERON;
        
    if(errorCode())
        return errorCode();
        
    if(info.mPKp < 0 || !std::isfinite(info.mPKp))
        return MC_ERRORCODE_CFGPKPILLEGAL;
        
    if(info.mFF < 0 || !std::isfinite(info.mFF))
        return MC_ERRORCODE_CFGFEEDFORWORDILLEGAL;
        
    switch(info.mControlMode) {
        case MC_CONTROLMODE_POSOPENLOOP:
        case MC_CONTROLMODE_VELCLOSELOOP:
        case MC_CONTROLMODE_VELOPENLOOP:
            break;
            
        default:
            return MC_ERRORCODE_CONTROLMODEILLEGAL;
    }
    
    mImpl_->mControl = info;
    
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode AxisBase::setHomePosition(double homePos)
{
    if(!std::isfinite(homePos))
        return MC_ERRORCODE_HOMEPOSITIONILLEGAL;
        
    double _2147 = fabs(mImpl_->toSystemLogic(2147483648.0));
    double _4294 = fabs(mImpl_->toSystemLogic(4294967296.0));
    
    if(homePos >= _2147) {
        homePos -= _4294;
    } else if(homePos < -_2147) {
        homePos += _4294;
    }
    
    printLog(MC_LOGLEVEL_DEBUG, 
        "Set home pos %lf, previous home pos %lf, diff %lf\n", 
        homePos, mImpl_->mHomePos, homePos - mImpl_->mHomePos);
    mImpl_->mHomePos = homePos;
    return MC_ERRORCODE_GOOD;
}

const AxisMetricInfo& AxisBase::metricInfo(void) const
{
    return mImpl_->mMetric;
}

const AxisRangeLimitInfo& AxisBase::rangeLimitInfo(void) const
{
    return mImpl_->mRangeLimit;
}

const AxisMotionLimitInfo& AxisBase::motionLimitInfo(void) const
{
    return mImpl_->mMotionLimit;
}

const AxisControlInfo& AxisBase::controlInfo(void) const
{
    return mImpl_->mControl;
}

double AxisBase::homePosition(void) const
{
    return mImpl_->mHomePos;
}

MC_ErrorCode AxisBase::setPower(
    bool powerStatus, 
    bool enablePositive, 
    bool enableNegative, 
    bool& isDone)
{
    isDone = false;
    
    if(errorCode())
        return errorCode();
    
    if(mImpl_->mPowerStatusValid && 
        (mImpl_->mPowerStatus == powerStatus)) {
        isDone = true;
        return MC_ERRORCODE_GOOD;
    }
    
    mImpl_->mPowerStatus = powerStatus;
    mImpl_->mEnablePositive = enablePositive;
    mImpl_->mEnableNegative = enableNegative;
    mImpl_->mPowerStatusValid = false;
    
    return MC_ERRORCODE_GOOD;
}

void AxisBase::emergStop(MC_ErrorCode errorCodeToSet)
{
    if(errorCode())
        return;
    
    mImpl_->mErrorCode = errorCodeToSet;
    mImpl_->mCmdVel = mImpl_->mCmdAcc = 0;
    mImpl_->mSubmitCmdPos = mImpl_->mCmdPos;
    mImpl_->mEncoderOverflowOffset = 0;
    mImpl_->mPowerStatusValid = false;
    mImpl_->mNeedReset = false;
    mImpl_->mServo->emergStop();
    printLog(MC_LOGLEVEL_WARN, 
        "EmergStop, ErrorID 0x%x, AxisErrorID 0x%x\n", 
        errorCode(), devErrorCode());
    URANUS_CALL_EVENT(onError, this, errorCode());
}

MC_ErrorCode AxisBase::resetError(bool& isDone)
{
    if(!errorCode()) {
        isDone = true;
        return MC_ERRORCODE_GOOD;
    }
    
    mImpl_->mNeedReset = true;
    return MC_ERRORCODE_GOOD;
}

MC_ErrorCode AxisBase::setPosition(double pos, double vel, double acc)
{
    if(errorCode())
        return errorCode();
    
    if(!powerStatus())
        return MC_ERRORCODE_AXISPOWEROFF;
        
    if(!std::isfinite(pos)) {
        emergStop(MC_ERRORCODE_POSINFINITY);
        return errorCode();
    }
    
    mImpl_->mSubmitCmdPos = pos;
    //mImpl_->mCmdVel = (mImpl_->mSubmitCmdPos - mImpl_->mCmdPos) * frequency();
    mImpl_->mCmdVel = vel;
    mImpl_->mCmdAcc = acc;
    return MC_ERRORCODE_GOOD;
}

const char* AxisBase::axisName(void) const
{
    return mImpl_->mAxisName;
}

bool AxisBase::powerStatus(void) const
{
    return mImpl_->mPowerStatus && mImpl_->mPowerStatusValid;
}

MC_ErrorCode AxisBase::errorCode(void) const
{
    return mImpl_->mErrorCode;
}

MC_ServoErrorCode AxisBase::devErrorCode(void) const
{
    return mImpl_->mDevErrorCode;
}

double AxisBase::cmdPosition(void) const
{
    return mImpl_->mCmdPos;
}

double AxisBase::cmdVelocity(void) const
{
    return mImpl_->mCmdVel;
}

double AxisBase::cmdAcceleration(void) const
{
    return mImpl_->mCmdAcc;
}

double AxisBase::positionOffset(void) const
{
    return mImpl_->mEncoderOverflowOffset;
}

double AxisBase::actPosition(void) const
{
    return mImpl_->toSystemLogic(mImpl_->mServo->pos());
}

double AxisBase::actVelocity(void) const
{
    return mImpl_->toSystemLogic(mImpl_->mServo->vel());
}

double AxisBase::actAcceleration(void) const
{
    return mImpl_->toSystemLogic(mImpl_->mServo->acc());
}

double AxisBase::actTorque(void) const
{
    return mImpl_->mServo->torque();
}

bool AxisBase::servoReadVal(int index, double& value)
{
    return mImpl_->mServo->readVal(index, value);
}

bool AxisBase::servoWriteVal(int index, double value)
{
    return mImpl_->mServo->writeVal(index, value);
}

double AxisBase::userPosToSys(
    double baseSysPos, double userPos, MC_Direction dir) const
{
    double userPosNoHome = mImpl_->stripHomeOffset(baseSysPos, userPos);
    return mImpl_->moduloToLinear(baseSysPos, userPosNoHome, dir);
}

double AxisBase::sysPosToUser(double sysPos) const
{
    return mImpl_->linearToModulo(mImpl_->packHomeOffset(sysPos));
}

void AxisBase::printLog(MC_LogLevel level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintLog(level, fmt, ap);
    va_end(ap);
}

}
