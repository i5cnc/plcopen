/*
 * axis_move.cpp
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
 * 示例代码，使用MC_Power、MC_MoveAbsolute使能轴并完成两段移动
 * MC_ReadActualPosition与ReadActualVelocity用于读取当前位置与速度
 * 
 */
 
#include "Scheduler.hpp"
#include "FbSingleAxis.hpp"
#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace Uranus;
using namespace std;

int main(void)
{
    cout.precision(8);
    
    //调度器初始化
    Scheduler sched;
    double frequency = 100; //定义外部周期调度的频率
    int32_t axisId = 1; //id可随意定义，各轴不重复即可
    sched.setFrequency(frequency);
    Axis* axis = sched.newAxis(axisId, new Servo());
    //new完成后也可通过 axis = sched.axis(axisId); 获取axis实例
    
    //功能块初始化
    FbPower power;
    power.mAxis = axis;
    power.mEnable = true;
    power.mEnablePositive = true;
    power.mEnableNegative = true;
    
    FbMoveAbsolute moveAbs1;
    moveAbs1.mAxis = axis;
    moveAbs1.mPosition = 500;
    moveAbs1.mVelocity = 400;
    moveAbs1.mAcceleration = 500;
    moveAbs1.mDeceleration = 500;
    
    FbMoveAbsolute moveAbs2;
    moveAbs2.mAxis = axis;
    moveAbs2.mPosition = 1000;
    moveAbs2.mVelocity = 200;
    moveAbs2.mAcceleration = 300;
    moveAbs2.mDeceleration = 300;
    moveAbs2.mBufferMode = MC_BUFFERMODE_BUFFERED;
    
    FbReadActualPosition readPos;
    readPos.mAxis = axis;
    readPos.mEnable = true;
    
    FbReadCommandVelocity readVel;
    readVel.mAxis = axis;
    readVel.mEnable = true;
    
    double t = 0;
    bool moveAbs1AlreadyDone = false;
    //“实时”周期任务
    while(1) {
         //调度器周期处理
        sched.runCycle();
        
        //功能块调用
        power.call();
        moveAbs1.call();
        moveAbs2.call();
        readPos.call();
        readVel.call();
        
        //显示当前时间，轴的位置与速度
        cout << "time:" << fixed << t 
            << ",\tposition:" << readPos.mPosition 
            << ",\tvelocity:" << readVel.mVelocity 
            << endl;
            
        bool isPowerOn = power.mStatus && power.mValid;
        if(isPowerOn && !moveAbs1.mExecute) { //使能成功后move1开始移动
            cout << "axis poweron, moveAbs1 start" << endl;
            moveAbs1.mExecute = isPowerOn;
        }
        
        if(moveAbs1.mDone && !moveAbs1AlreadyDone) { //提示move1完成
            cout << "moveAbs1 complete, moveAbs2 start" << endl;
            moveAbs1AlreadyDone = true;
        }
        
        moveAbs2.mExecute = moveAbs1.mBusy;
        
        if(moveAbs2.mDone) { //提示move2完成并退出
            cout << "moveAbs2 complete" << endl;
            break;
        }
        
        usleep(1000000 / frequency); //演示用，sleep代替实时定时器
        t += 1 / frequency;
    }
    
    return 0;
}
