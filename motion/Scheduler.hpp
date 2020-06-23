/*
 * Scheduler.hpp
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

#ifndef _URANUS_SCHEDULER_HPP_
#define _URANUS_SCHEDULER_HPP_

#include "Global.hpp"
#include "Servo.hpp"

namespace Uranus {

#pragma pack(push)
#pragma pack(4)
    
class Axis;
class Scheduler
{
public:
    Scheduler();
    
    virtual ~Scheduler();
    
    //执行一次插补
    void runCycle(void);
    
    //设定runCycle的频率
    MC_ErrorCode setFrequency(double frequency);
    
    //获取频率
    double frequency(void) const; 
    
    //当前tick，每次runCycle后自增
    uint32_t tick(void) const;
    
    /*
     * 新建轴
     * axisId:轴Id，不重复
     * servo:伺服驱动器实例
     * 返回:轴实例
     */
    Axis* newAxis(int32_t axisId, Servo* servo);
        
    //通过Id获取轴
    Axis* axis(int32_t axisId) const;
    
    //设定轴配置
    MC_ErrorCode setAxisConfig(Axis* axis, const AxisConfig& config);
    
    //直接设定轴零点配置
    MC_ErrorCode setAxisHomePosition(Axis* axis, double homePos);
    
    //获取第一个轴
    Axis* axisListFirst(void) const;
    
    //获取下一个轴
    Axis* axisListNext(const Axis* one) const;
    
    //释放所有创建的轴
    void release(void);
    
protected:
    virtual void vprintLog(MC_LogLevel level, const char* fmt, va_list ap) { }
    
private:
    class SchedulerImpl;
    SchedulerImpl* mImpl_;
    friend class Axis;
};

#pragma pack(pop)

}

#endif /** _URANUS_SCHEDULER_HPP_ **/
    
