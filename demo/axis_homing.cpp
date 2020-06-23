/*
 * axis_homing.cpp
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
 * 示例代码，使用MC_Power、MC_Home使能轴并完成回零
 * MC_ReadActualPosition与ReadActualVelocity用于读取当前位置与速度
 * MyAxis继承Servo类仿真实现回零点在某位置被触发与回弹的电平变化
 * 
 */
 
#include "Scheduler.hpp"
#include "FbSingleAxis.hpp"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdio.h>

using namespace Uranus;
using namespace std;

class MyServo : public Servo
{
public:
    uint8_t homingSwitch = false; //定义“物理”回零点
    int32_t homingSwitchPosition = 50 * 8192; //定义回零点的“编码器位置”

protected:
    virtual void runCycle(double freq) override;
};

void MyServo::runCycle(double freq)
{
    Servo::runCycle(freq);
    //当前“编码器位置”超出homingSwitchPosition点，则回零点置为true,反之置为false
    homingSwitch = (pos() >= homingSwitchPosition);
}

int main(void)
{
    cout.precision(8);
    
    //调度器初始化
    Scheduler sched;
    double frequency = 100; //定义外部周期调度的频率
    int32_t axisId = 1; //id可随意定义，各轴不重复即可
    sched.setFrequency(frequency);
    MyServo* servo = new MyServo();
    Axis* axis = sched.newAxis(axisId, servo);
    //设定axis的回零开关地址与回零参数
    AxisConfig config;
    config.mHomingInfo.mHomingSig = &servo->homingSwitch;
    config.mHomingInfo.mHomingMode = MC_HOMINGMODE_MODE7;
    config.mHomingInfo.mHomingVelSearch = 20;
    config.mHomingInfo.mHomingVelRegression = 1;
    config.mHomingInfo.mHomingAcc = 50;
    sched.setAxisConfig(axis, config);
    
    //功能块初始化
    FbPower power;
    power.mAxis = axis;
    power.mEnable = true;
    power.mEnablePositive = true;
    power.mEnableNegative = true;
    
    FbHome home;
    home.mAxis = axis;
    home.mPosition = 200; //零点位置指定为位置200
    
    FbReadActualPosition readPos;
    readPos.mAxis = axis;
    readPos.mEnable = true;
    
    FbReadCommandVelocity readVel;
    readVel.mAxis = axis;
    readVel.mEnable = true;
    
    double t = 0;
    //“实时”周期任务
    while(1) {
         //调度器周期处理
        sched.runCycle();
        
        //功能块调用
        power.call();
        home.call();
        readPos.call();
        readVel.call();
        
        //显示当前时间，轴的位置与速度
        cout << fixed << t << " "
            << readPos.mPosition << " "
            << readVel.mVelocity << " "
            << endl;
            
        bool isPowerOn = power.mStatus && power.mValid;
        if(isPowerOn && !home.mExecute) { //使能成功后开始回零
            cout << "axis poweron, homing start" << endl;
            home.mExecute = isPowerOn;
        }
        
        if(home.mDone) { //提示回零完成并退出
            cout << "homing complete" << endl;
            break;
        }
        
        usleep(1000000 / frequency); //演示用，sleep代替实时定时器
        t += 1 / frequency;
    }
    
    return 0;
}
