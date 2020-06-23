/*
 * ProfilePlanner.hpp
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

/* lagacy code */

#ifndef _URANUS_PROFILEPLANNER_HPP_
#define _URANUS_PROFILEPLANNER_HPP_

#include <stdint.h>

namespace Uranus
{

#define MAX_ROUTE_SEGMENT_NUM 5

class ProfilePlanner
{
public:
    typedef struct
    {
        double start_position;
        double end_position;
        double start_vel;
        double end_vel;
    }InputInfo;
    
    typedef struct
    {
        double start_position;
        double end_position;
        double start_vel;
        double acc;
        double t;
        int32_t tick;
        int magic_flags;
    }Segment;
    
    typedef struct
    {
        double position;
        double velocity;
        int32_t current_tick;
        uint32_t frequency;
        uint32_t current_segment;
        uint32_t number_segment;
        double t_remain;
        Segment segments[MAX_ROUTE_SEGMENT_NUM];
    }ProfilePlannerData;
    
public:
    ProfilePlannerData data;
    ProfilePlannerData data_backup;
    InputInfo input_info;
    uint32_t frequency;
    
public:
    ProfilePlanner();
    virtual ~ProfilePlanner(){}
    
    bool setFrequency(uint32_t frequency);
    
    static double limitStartVel(double dist, double start_vel, double end_vel, double dec);
    
    static double calculateDist(double start_vel, double end_vel, double acc, double dec);

    bool plan(
        double start_position, double end_position, 
        double start_vel, double vel, double end_vel,
        double acc, double dec);
        
    bool execute(void);
    
    /** 
     *  return
     *  0: standstill
     *  1: constant velocity
     *  2: accelerating
     *  3: decelerating
     **/
    int readStatus(void);
    
    double getPosition(void);
    
    double getVelocity(void);
    
    double getAcceleration(void);
    
    double getStartPosition(void);
    
    double getEndPosition(void);
    
    double getStartVelocity(void);
    
    double getEndVelocity(void);
    
    void resetRemain(uint32_t freq);
    
    void setPositionOffset(double pos);
    
private:
    void pushData(void);
    
    void popData(void);
};

void print_all(ProfilePlanner::Segment* segments, int num = 5);

}

#endif /** _URANUS_PROFILEPLANNER_HPP_ **/
