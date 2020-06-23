/*
 * AxisBase.hpp
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

#ifndef _URANUS_AXISBASE_HPP_
#define _URANUS_AXISBASE_HPP_

#include "Global.hpp"
#include "Event.hpp"
#include <stdarg.h>

namespace Uranus {

class Servo;
class AxisBase
{
public:
    AxisBase();
    virtual ~AxisBase();

    void runCycle(void);
    
    void setServo(Servo* servo);
    
    void setAxisName(const char* name);
    MC_ErrorCode setMetricInfo(const AxisMetricInfo& info);
    MC_ErrorCode setRangeLimitInfo(const AxisRangeLimitInfo& info);
    MC_ErrorCode setMotionLimitInfo(const AxisMotionLimitInfo& info);
    MC_ErrorCode setControlInfo(const AxisControlInfo& info);
    MC_ErrorCode setHomePosition(double homePos);
    
    const char* axisName(void) const;
    const AxisMetricInfo& metricInfo(void) const;
    const AxisRangeLimitInfo& rangeLimitInfo(void) const;
    const AxisMotionLimitInfo& motionLimitInfo(void) const;
    const AxisControlInfo& controlInfo(void) const;
    double homePosition(void) const;
    
    MC_ErrorCode setPower(
        bool powerStatus, 
        bool enablePositive, 
        bool enableNegative, 
        bool& isDone);
        
    void emergStop(MC_ErrorCode errorCodeToSet);
    MC_ErrorCode resetError(bool& isDone);
    MC_ErrorCode setPosition(double pos, double vel, double acc);
    
    bool powerStatus(void) const;
    MC_ErrorCode errorCode(void) const;
    MC_ServoErrorCode devErrorCode(void) const;
    
    double cmdPosition(void) const;
    double cmdVelocity(void) const;
    double cmdAcceleration(void) const;
    double positionOffset(void) const;
    
    double actPosition(void) const;
    double actVelocity(void) const;
    double actAcceleration(void) const;
    double actTorque(void) const;
    
    bool servoReadVal(int index, double& value);
    bool servoWriteVal(int index, double value);
    
    double userPosToSys(double baseSysPos, double userPos, MC_Direction dir) const;
    double sysPosToUser(double sysPos) const;
    
    void printLog(MC_LogLevel level, const char* fmt, ...);
    
protected: //事件通知
    URANUS_DEFINE_EVENT(onError, AxisBase*, MC_ErrorCode);
    URANUS_DEFINE_EVENT(onPowerStatusChanged, AxisBase*, bool);
    URANUS_DEFINE_EVENT(onPositionOffset, AxisBase*, double);

protected:
    virtual double frequency(void) = 0;
    virtual uint32_t tick(void) = 0;
    virtual void vprintLog(MC_LogLevel level, const char* fmt, va_list ap) = 0;
    
private:
    class AxisBaseImpl;
    AxisBaseImpl* mImpl_;
};

}

#endif /** _URANUS_AXISBASE_HPP_ **/
