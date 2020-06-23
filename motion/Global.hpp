/*
 * Global.hpp
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

#ifndef _URANUS_MOTION_GLOBAL_HPP_
#define _URANUS_MOTION_GLOBAL_HPP_

#include <cstdint>
#include <cstddef>

namespace Uranus {
    
#pragma pack(push)
#pragma pack(4)
    
#define URANUS_AXESGROUP_IDENT_NUM 8
#define URANUS_CARTESIAN_DIMENSION3 3
#define URANUS_CARTESIAN_DIMENSION6 6
#define URANUS_TRANSITIONPARAMETER_NUM 4

typedef uint32_t MC_ServoErrorCode;

typedef enum {
    MC_SERVOCONTROLMODE_POSITION    = 0,
    MC_SERVOCONTROLMODE_VELOCITY    = 1,
    MC_SERVOCONTROLMODE_TORQUE      = 2,
}MC_ServoControlMode;
    
typedef enum 
{
    MC_ERRORCODE_GOOD                           = 0x0, //成功
    
    MC_ERRORCODE_QUEUEFULL                      = 0x1, //轴队列已满
    MC_ERRORCODE_AXISENCODEROVERFLOW            = 0x2, //轴编码器溢出
    MC_ERRORCODE_AXISPOWEROFF                   = 0x3, //轴未使能
    MC_ERRORCODE_AXISPOWERON                    = 0x4, //轴已功能
    MC_ERRORCODE_FREQUENCYILLEGAL               = 0x5, //频率不合法
    MC_ERRORCODE_AXISNOTEXIST                   = 0x8, //轴ID号不存在
    MC_ERRORCODE_AXISBUSY                       = 0xA, //轴正忙，有功能块正在控制轴运动
    MC_ERRORCODE_FAILEDTOBUFFER                 = 0xF, //不支持以buffer形式添加
    MC_ERRORCODE_BLENDINGMODEILLEGAL            = 0x10, //BufferMode值非法
    MC_ERRORCODE_PARAMETERNOTSUPPORT            = 0x14, //不支持该参数号
    MC_ERRORCODE_OVERRIDEILLEGAL                = 0x17, //OVERRIDE值非法
    MC_ERRORCODE_SHIFTINGMODEILLEGAL            = 0x19, //移动模式非法
    MC_ERRORCODE_SOURCEILLEGAL                  = 0x1A, //获取源非法
    MC_ERRORCODE_CONTROLMODEILLEGAL             = 0x23, //控制模式设置错误

    MC_ERRORCODE_POSILLEGAL                     = 0x100, //位置不合法
    MC_ERRORCODE_ACCILLEGAL                     = 0x101, //加/减速度不合法
    MC_ERRORCODE_VELILLEGAL                     = 0x102, //速度不合法
    MC_ERRORCODE_AXISHARDWARE                   = 0x103, //硬件错误
    MC_ERRORCODE_VELLIMITTOOLOW                 = 0x104, //由于配置文件限制，无法到达跟随的速度（电子齿轮，凸轮中）
    MC_ERRORCODE_ENDVELCANNOTREACH              = 0x105, //实际终速度过高无法到达预设速度

    MC_ERRORCODE_CMDPPOSOVERLIMIT               = 0x106, //指令位置超出配置文件正向限制
    MC_ERRORCODE_CMDNPOSOVERLIMIT               = 0x107, //指令位置超出配置文件负向限制
    MC_ERRORCODE_FORBIDDENPPOSMOVE              = 0x108, //禁止正向移动
    MC_ERRORCODE_FORBIDDENNPOSMOVE              = 0x109, //禁止负向移动

    MC_ERRORCODE_POSLAGOVERLIMIT                = 0x10A, //轴跟随误差超限
    MC_ERRORCODE_CMDVELOVERLIMIT                = 0x10B, //轴指令速度超出限制
    MC_ERRORCODE_CMDACCOVERLIMIT                = 0x10C, //轴指令加速度超出限制
    MC_ERRORCODE_POSINFINITY                    = 0x10E, //轴设定位置不合法
    
    MC_ERRORCODE_SOFTWAREEMGS                   = 0x1EE, //用户急停
    MC_ERRORCODE_SYSTEMEMGS                     = 0x1EF, //系统急停
    MC_ERRORCODE_COMMUNICATION                  = 0x1F0, //硬件通信异常

    /** 配置错误**/
    MC_ERRORCODE_CFGAXISIDILLEGAL               = 0x201,
    MC_ERRORCODE_CFGUNITRATIOOUTOFRANGE         = 0x202,
    MC_ERRORCODE_CFGCONTROLMODEILLEGAL          = 0x203,
    MC_ERRORCODE_CFGVELLIMITILLEGAL             = 0x204,
    MC_ERRORCODE_CFGACCLIMITILLEGAL             = 0x205,
    MC_ERRORCODE_CFGPOSLAGILLEGAL               = 0x206,
    MC_ERRORCODE_CFGPKPILLEGAL                  = 0x207,
    MC_ERRORCODE_CFGFEEDFORWORDILLEGAL          = 0x208,
    MC_ERRORCODE_CFGMODULOILLEGAL               = 0x209,
    
    MC_ERRORCODE_HOMINGVELILLEGAL               = 0x210,
    MC_ERRORCODE_HOMINGACCILLEGAL               = 0x211,
    MC_ERRORCODE_HOMINGSIGILLEGAL               = 0x212,
    MC_ERRORCODE_HOMINGMODEILLEGAL              = 0x214,
    MC_ERRORCODE_HOMEPOSITIONILLEGAL            = 0x215,

    MC_ERRORCODE_AXISDISABLED                   = 0x500,
    MC_ERRORCODE_AXISSTANDSTILL                 = 0x501,
    MC_ERRORCODE_AXISHOMING                     = 0x502,
    MC_ERRORCODE_AXISDISCRETEMOTION             = 0x503,
    MC_ERRORCODE_AXISCONTINUOUSMOTION           = 0x504,
    MC_ERRORCODE_AXISSYNCHRONIZEDMOTION         = 0x505,
    MC_ERRORCODE_AXISSTOPPING                   = 0x506,
    MC_ERRORCODE_AXISERRORSTOP                  = 0x507,
}MC_ErrorCode;

typedef enum
{
    MC_AXISSTATUS_DISABLED              = 0,
    MC_AXISSTATUS_STANDSTILL            = 1,
    MC_AXISSTATUS_HOMING                = 2,
    MC_AXISSTATUS_DISCRETEMOTION        = 3,
    MC_AXISSTATUS_CONTINUOUSMOTION      = 4,
    MC_AXISSTATUS_SYNCHRONIZEDMOTION    = 5,
    MC_AXISSTATUS_STOPPING              = 6,
    MC_AXISSTATUS_ERRORSTOP             = 7,
}MC_AxisStatus;

typedef enum
{
    MC_GROUPSTATUS_DISABLED     = 0,
    MC_GROUPSTATUS_STANDBY      = 1,
    MC_GROUPSTATUS_HOMING       = 2,
    MC_GROUPSTATUS_MOVING       = 3,
    MC_GROUPSTATUS_STOPPING     = 4,
    MC_GROUPSTATUS_ERRORSTOP    = 5,
}MC_GroupStatus;

typedef enum
{
    MC_MOTIONSTATE_INPOSITION       = 0,
    MC_MOTIONSTATE_CONSTANTVELOCITY = 1,
    MC_MOTIONSTATE_ACCELERATING     = 2,
    MC_MOTIONSTATE_DECELERATING     = 3,
}MC_MotionState;

typedef enum
{
    MC_BUFFERMODE_ABORTING          = 0,
    MC_BUFFERMODE_BUFFERED          = 1,
    MC_BUFFERMODE_BLENDINGLOW       = 2,
    MC_BUFFERMODE_BLENDINGPREVIOUS  = 3,
    MC_BUFFERMODE_BLENDINGNEXT      = 4,
    MC_BUFFERMODE_BLENDINGHIGH      = 5,
    MC_BUFFERMODE_BLENDINGCNC       = 128,
}MC_BufferMode;

typedef enum
{
    MC_DIRECTION_POSITIVE       = 1,
    MC_DIRECTION_SHORTESTWAY    = 2,
    MC_DIRECTION_NEGATIVE       = 3,
    MC_DIRECTION_CURRENT        = 4,
}MC_Direction;

typedef enum
{
    MC_SOURCE_SETVALUE          = 0,
    MC_SOURCE_ACTUALVALUE       = 1,
}MC_Source;

typedef enum
{
    MC_SHIFTINGMODE_ABSOLUTE   = 0,
    MC_SHIFTINGMODE_RELATIVE   = 1,
    MC_SHIFTINGMODE_ADDITIVE   = 2,
}MC_ShiftingMode;

typedef enum
{
    MC_HOMINGMODE_DIRECT = 1000, //直接以当前位置作为零点
    
    MC_HOMINGMODE_MODE5  = 1005, //负向移动寻找回零开关，触发后正向移动离开回零开关，最终停留在刚离开回零开关处，回零开关为上升沿触发
    MC_HOMINGMODE_MODE6  = 1006, //负向移动寻找回零开关，触发后正向移动离开回零开关，最终停留在刚离开回零开关处，回零开关为下降沿触发
    MC_HOMINGMODE_MODE7  = 1007, //正向移动寻找回零开关，触发后负向移动离开回零开关，最终停留在刚离开回零开关处，回零开关为上升沿触发
    MC_HOMINGMODE_MODE8  = 1008, //正向移动寻找回零开关，触发后负向移动离开回零开关，最终停留在刚离开回零开关处，回零开关为下降沿触发
}MC_HomingMode;

typedef enum
{
    MC_TOUCHPROBESTATUS_NOTEXIST    = 0,
    MC_TOUCHPROBESTATUS_RESETTING   = 1,
    MC_TOUCHPROBESTATUS_RESETED     = 2,
    MC_TOUCHPROBESTATUS_TIGGERING   = 3,
    MC_TOUCHPROBESTATUS_TIGGERED    = 4,
}MC_TouchProbeStatus;

typedef enum 
{
    MC_CONTROLMODE_POSOPENLOOP  = 0,
    MC_CONTROLMODE_VELCLOSELOOP = 1,
    MC_CONTROLMODE_VELOPENLOOP  = 2,
}MC_ControlMode;

typedef enum
{
    MC_COORDSYSTEM_ACS    = 0,
    MC_COORDSYSTEM_MCS    = 1,
    MC_COORDSYSTEM_PCS    = 2,
}MC_CoordSystem;

typedef enum
{
    MC_CIRCMODE_BORDER  = 0,
    MC_CIRCMODE_CENTER  = 1,
    MC_CIRCMODE_RADIUS  = 2,
    MC_CIRCMODE_VECTOR  = 1000,
}MC_CircMode;

typedef enum
{
    MC_CIRCPATH_CLOCKWISE           = 0,
    MC_CIRCPATH_COUNTERCLOCKWISE    = 1,
}MC_CircPath;

typedef enum
{
    MC_TRANSITIONMODE_NONE                 = 0,
    MC_TRANSITIONMODE_STARTVELOCITY        = 1, //不支持
    MC_TRANSITIONMODE_CONSTANTVELOCITY     = 2, //不支持
    MC_TRANSITIONMODE_CORNERDISTANCE       = 3,
    MC_TRANSITIONMODE_MAXCORNERDEVIATION   = 4,
}MC_TransitionMode;

typedef enum
{
    MC_LOGLEVEL_ERROR   = 0,
    MC_LOGLEVEL_WARN    = 1,
    MC_LOGLEVEL_INFO    = 2,
    MC_LOGLEVEL_DEBUG   = 3,
}MC_LogLevel;

//////////////////////////////////////////////////////////////

struct AxisMetricInfo
{
    double mDevUnitRatio = 8192;                //设备编码器单位比率
    double mModulo = 0;                         //模数值
};

struct AxisRangeLimitInfo
{
    bool mSwLimitPositive = false;              //正向限位启用标志位
    bool mSwLimitNegative = false;              //负向限位启用标志位
    double mLimitPositive = 0;                  //正向限位位置
    double mLimitNegative = 0;                  //负向限位位置
};

struct AxisMotionLimitInfo
{
    double mVelLimit = 1000;                    //速度限制
    double mAccLimit = 5000;                    //加速度限制
    double mPosLagLimit = 150;                  //跟随误差限制
};

struct AxisControlInfo
{
    MC_ControlMode mControlMode = MC_CONTROLMODE_POSOPENLOOP;   //控制模式
    double mPKp = 10;                                           //闭环位置Kp
    double mFF = 0;                                             //位置前馈
};

struct AxisHomingInfo
{
    uint8_t* mHomingSig = 0;                            //回零信号地址
    uint8_t mHomingSigBitOffset = 0;                    //回零信号偏移
    MC_HomingMode mHomingMode = MC_HOMINGMODE_DIRECT;   //回零模式
    double mHomingVelSearch = 0;                        //寻找回零信号速度
    double mHomingVelRegression = 0;                    //返回零位速度
    double mHomingAcc = 0;                              //回零加速度
    double mHomingJerk = 0;                             //回零加加速
};

struct AxisConfig
{
    AxisMetricInfo mMetricInfo;
    AxisRangeLimitInfo mRangeLimitInfo;
    AxisMotionLimitInfo mMotionLimitInfo;
    AxisControlInfo mControlInfo;
    AxisHomingInfo mHomingInfo;
};

//////////////////////////////////////////////////////////////
/*
struct GroupMotionLimitInfo
{
    double mLinVelLimit = 1000;     //线速度限制
    double mLinAccLimit = 5000;     //线加速度限制
    double mAngVelLimit = 3600;     //角速度限制
    double mAngAccLimit = 18000;    //角加速度限制
};*/

struct GroupConfig
{
//    GroupMotionLimitInfo mMotionLimitInfo;
};

#pragma pack(pop)

}

#endif /** _URANUS_MOTION_GLOBAL_HPP_ **/
