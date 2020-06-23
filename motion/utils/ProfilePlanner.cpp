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

#include "ProfilePlanner.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "MathUtils.hpp"

//#define MC_DEBUG 1

namespace Uranus
{

typedef ProfilePlanner::Segment Segment;
typedef ProfilePlanner::ProfilePlannerData ProfilePlannerData;

static int route_calculate(Segment* segments, 
    double shift, double start_vel, double vel, 
    double acc, double dec, double& end_vel);
    
static bool route_param_pre_process(
    double& vel, double& acc, double& dec, double& end_vel);

static int route_verify_and_shift(
    Segment* segments, double end_vel, double pos_offset);

static bool check_results(const Segment* segments, int length);

static void route_discretization(
    Segment* segments, int length, uint32_t frequency, double& prev_t_remain);
    
static int tiny_segment_merge(
    Segment* segments, int length, uint32_t frequency, int32_t& new_start_tick);

static double cal_shift(
    double start_vel, double end_vel, double acc, double* t);
 
static inline bool is_acc_neg(double start_vel, double end_vel);

void print_all(Segment* segments, int num)
{
    for(int i=0; i<num; ++i)
    {
        printf(
            "segment %d: "
            "start_position=%lf, "
            "end_position=%lf, "
            "start_vel=%lf, "
            "acc=%lf, "
            "t=%lf\n",
            i,
            segments[i].start_position,
            segments[i].end_position,
            segments[i].start_vel,
            segments[i].acc,
            segments[i].t);
    }
}

#define set_route_segment(Segment, StartPos, EndPos, StartVel, Acc, T) \
{ (Segment)->start_position = (StartPos); \
    (Segment)->end_position = (EndPos); \
    (Segment)->start_vel = (StartVel); \
    (Segment)->acc = (Acc); (Segment)->t = (T); }
    
ProfilePlanner::ProfilePlanner()
{
    memset(this, 0, sizeof(ProfilePlanner));
    frequency = 1000;
}

bool ProfilePlanner::setFrequency(uint32_t _frequency)
{
    if(!_frequency || _frequency > 100000)
        return false;
    
    frequency = _frequency;
    return true;
}

double ProfilePlanner::limitStartVel(double dist, double start_vel, double end_vel, double dec)
{
    if(start_vel == 0.0)
        return start_vel;
        
    if(((start_vel<0.0) != (end_vel<0.0)) && (end_vel != 0.0))
        return start_vel;
        
    double vel_limit = sqrt(2 * fabs(dist) * fabs(dec) + __square(end_vel));
    
    if(vel_limit < fabs(start_vel))
        start_vel = (start_vel>0.0)? vel_limit: -vel_limit;
    
    return start_vel;
}

double ProfilePlanner::calculateDist(double start_vel, double end_vel, double acc, double dec)
{
    acc = fabs(acc);
    dec = fabs(dec);
    
    if(isOpposite(start_vel, end_vel))
        return (start_vel * fabs(start_vel) / dec + end_vel * fabs(end_vel) / acc) * 0.5;
    else
    {
        double cc = (fabs(start_vel) > fabs(end_vel))? dec: acc;
        return (start_vel + end_vel) * fabs(start_vel - end_vel) / cc * 0.5;
    }
}

bool ProfilePlanner::plan(
        double start_position, double end_position, 
        double start_vel, double vel, double end_vel,
        double acc, double dec)
{
    double shift;
    int route_calculate_result;
    
    if(__iseq(start_position, end_position) && __iseq(start_vel, end_vel))
    {
        data.number_segment = 0;
        goto EXIT;
    }
        
    if(!route_param_pre_process(vel, acc, dec, end_vel))
        return false;
        
#ifdef MC_DEBUG
    printf(
        "get a cmd: pos %lf -> %lf, vel %lf -> %lf -> %lf, acc: %lf, dec: %lf\n",
        start_position, end_position, start_vel, vel, end_vel, acc, dec);
#endif
    
    pushData();
    
    data.current_tick = 0;
    data.current_segment = 0;
    memset(data.segments, 0, sizeof(Segment) * MAX_ROUTE_SEGMENT_NUM);
    shift = end_position - start_position;
    
    route_calculate_result = route_calculate(data.segments, shift, start_vel, vel, acc, dec, end_vel);
        
    switch(route_calculate_result)
    {
        case 0: //all success
        case 1: //real end_vel < given end_vel
            break;
        case 2: //real end_vel > given end_vel
            popData();
            return false;
    }
    
    data.number_segment = 
        route_verify_and_shift(data.segments, end_vel, start_position);
    
    if(!check_results(data.segments, data.number_segment))
    {
        popData();
        return false;
    }
    
    if(data_backup.current_segment < data_backup.number_segment)
        data.t_remain = 1.0 / frequency;
    
    route_discretization(data.segments, data.number_segment, frequency, data.t_remain);
    
    data.number_segment = 
        tiny_segment_merge(data.segments, data.number_segment, frequency, data.current_tick);

#ifdef MC_DEBUG
    print_all(data.segments, data.number_segment);
#endif 
    
EXIT:
    data.frequency = frequency;
    data.position = start_position;
    data.velocity = start_vel;
    
    input_info.start_position = start_position;
    input_info.end_position = end_position;
    input_info.start_vel = start_vel;
    input_info.end_vel = end_vel;
    
    if(!data.number_segment)
        return false;
        
    if(route_calculate_result == 1)
        return false;
        
    return true;
}

bool ProfilePlanner::execute(void)
{
    if(data.current_segment >= data.number_segment)
    {
        data.velocity = input_info.end_vel;
        data.position += data.velocity / data.frequency;
        input_info.end_position = data.position;
        data.t_remain = 1.0 / data.frequency;
        return true;
    }
    
    double t = (double)(data.current_tick) / data.frequency;
    
    double acc_2 = data.segments[data.current_segment].acc * t / 2;
    
    data.velocity = data.segments[data.current_segment].start_vel + acc_2;
    data.position = data.segments[data.current_segment].start_position + data.velocity * t;
    data.velocity += acc_2;
    
    if(data.current_tick == data.segments[data.current_segment].tick && 
        !data.segments[data.current_segment].magic_flags)
    {
        goto SEGMENT_SUCCESS;
    }
    else if(data.current_tick > data.segments[data.current_segment].tick)
    {
        data.position = data.segments[data.current_segment].end_position;
        
        if(data.segments[data.current_segment].magic_flags)
        {
            data.t_remain = 0.0;
            data.velocity = 0;
        }

        goto SEGMENT_SUCCESS;
    }
    ++data.current_tick;
    
    return false;
    
SEGMENT_SUCCESS:
    data.current_tick = 0;
    
    if(++data.current_segment == data.number_segment)
        return true;
    else
        return false;
}

int ProfilePlanner::readStatus(void)
{
    if((data.current_segment >= data.number_segment) || 
        (data.segments[data.current_segment].acc == 0.0))
    {
        if(data.velocity == 0.0)
            return 0;
        else
            return 1;
    }
    
    if((data.segments[data.current_segment].acc > 0.0) != (data.velocity > 0.0))
        return 3;
    else
        return 2;
}

double ProfilePlanner::getPosition(void)
{
    return data.position;
}
    
double ProfilePlanner::getVelocity(void)
{
    return data.velocity;
}

double ProfilePlanner::getAcceleration(void)
{
    if(data.current_segment >= data.number_segment)
    {
        return 0;
    } 
    else
    {
        return data.segments[data.current_segment].acc;
    }
}

double ProfilePlanner::getStartPosition(void)
{
    return input_info.start_position;
}
    
double ProfilePlanner::getEndPosition(void)
{
    return input_info.end_position;
}

void ProfilePlanner::setPositionOffset(double pos)
{
    if(data.current_segment >= data.number_segment)
    {
        data.position += pos;
        input_info.end_position = data.position;
    }
}

double ProfilePlanner::getStartVelocity(void)
{
    return input_info.start_vel;
}
    
double ProfilePlanner::getEndVelocity(void)
{
    return input_info.end_vel;
}

void ProfilePlanner::resetRemain(uint32_t freq)
{
    data.t_remain = 1.0 / freq;
}

void ProfilePlanner::pushData(void)
{
    data_backup = data;
}

void ProfilePlanner::popData(void)
{
    data = data_backup;
}

void ProfilePlanner::popData(void);

static int route_calculate(Segment* segments, 
    double shift, double start_vel, double vel, 
    double acc, double dec, double& end_vel)
{
    int result = 0;
    double shift_tmp = shift;
    double end_vel_tmp = end_vel;
    double shift_pre = 0.0;
        
#if 0
    //set velocity arrow
    if(isOpposite(start_vel, end_vel))
    {    
        double t_reverse_shift;
        double reverse_shift_point =
            cal_shift(start_vel, 0, dec, &t_reverse_shift) + 
            cal_shift(end_vel, 0, acc, &t_reverse_shift);
            
        if(shift < reverse_shift_point)
            vel = -vel;
    }
    else
    {
    }
#endif
    
    // imperfect 
    if((shift < 0.0) || (!shift && (end_vel < start_vel)))
        vel = -vel;
        
    /**
     * if shift and start_vel are not at the same direction, 
     * dec to 0 in advance.
     **/
    if(isOpposite(shift_tmp, start_vel))
    {
        double t;
        shift_pre = cal_shift(start_vel, 0.0, dec, &t);
        
        set_route_segment(segments, 
            0.0, shift_pre, start_vel, (shift_pre<0)? dec: -dec, t);
        
        start_vel = 0.0;
        shift_tmp -= shift_pre;
    }
    else
    {
        set_route_segment(segments, 0.0, 0.0, 0.0, 0.0, 0.0);
    }
    
    double v_uni = vel;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    double t_acc1 = 0.0, t_acc2 = 0.0, t_acc3 = 0.0, t_uni = 0.0;
    double s_acc1 = 0.0, s_acc2 = 0.0, s_acc3 = 0.0, s_uni = 0.0;
    double s_sum;
    
    /**
     * if vel and end_vel are not at the same direction, 
     * cal the last direction info in advance.
     **/
    if(isOpposite(vel, end_vel))
    {
        acc3 = is_acc_neg(vel, end_vel)? -acc: acc;
        s_acc3 = cal_shift(0.0, end_vel, acc3, &t_acc3);
        shift_tmp -= s_acc3;
        end_vel_tmp = 0.0;
    }
    
    //determine whether inputs param overtravel
    acc2 = (fabs(start_vel) > fabs(end_vel_tmp))? dec: acc;
    if(is_acc_neg(start_vel, end_vel_tmp))   {acc2 = -acc2;}
    s_acc2 = cal_shift(start_vel, end_vel_tmp, acc2, &t_acc2);
    
    if(__isne(s_acc2, shift_tmp) && (fabs(s_acc2) > fabs(shift_tmp)))
    {
        /** overtravel, change end_vel **/
        end_vel_tmp = sqrt(__square(start_vel) + 2 * acc2 * shift_tmp);
        
        if(fabs(end_vel) > end_vel_tmp)
            result = 1;
        else
            result = 2;
        
        if(shift_tmp < 0.0)   {end_vel_tmp = -end_vel_tmp;}
        end_vel = end_vel_tmp;
        //v_uni = start_vel; 喵喵喵？
        t_acc2 = (end_vel - start_vel) / acc2;
        s_acc2 = shift_tmp;
    }
    
    /*****************************************/
    if(fabs(start_vel) < fabs(vel))
        acc1 = acc;
    else
        acc1 = dec;
    
    if(is_acc_neg(start_vel, vel))
        acc1 = -acc1;
    /*****************************************/
    if(fabs(vel) < fabs(end_vel_tmp))
        acc2 = acc;
    else
        acc2 = dec;
        
    if(is_acc_neg(vel, end_vel_tmp))
        acc2 = -acc2;
    /*****************************************/
    
    s_acc1 = cal_shift(start_vel, vel, acc1, &t_acc1);
    s_acc2 = cal_shift(vel, end_vel_tmp, acc2, &t_acc2);
    
    s_sum = s_acc1 + s_acc2;
    
    // v_uni != vel, calculate new values
    if(fabs(s_sum) > fabs(shift_tmp) && __isne(s_sum, shift_tmp))
    {
        v_uni = sqrt(
            ((2 * shift_tmp + (start_vel * start_vel / acc1) - 
            (end_vel_tmp * end_vel_tmp / acc2)) * 
            (acc1 * acc2)) / (acc2 - acc1));
            
        if(vel < 0.0)   {v_uni = -v_uni;}
        s_acc1 = cal_shift(start_vel, v_uni, acc1, &t_acc1);
        s_acc2 = cal_shift(v_uni, end_vel_tmp, acc2, &t_acc2);
    }
    else
    {
        s_uni = shift_tmp - s_sum;
        t_uni = fabs(s_uni / vel);
    }
    
    shift_tmp = shift_pre;
    
    set_route_segment(segments + 1, 
        shift_tmp, shift_pre + s_acc1, start_vel, acc1, t_acc1);
        
    shift_tmp = shift_pre + s_acc1;
    
    set_route_segment(segments + 2, 
        shift_tmp, shift_tmp + s_uni, v_uni, 0.0, t_uni);
    
    shift_tmp += s_uni;
        
    set_route_segment(segments + 3, 
        shift_tmp, shift_tmp + s_acc2, v_uni, acc2, t_acc2);
        
    shift_tmp += s_acc2;

    set_route_segment(segments + 4, 
        shift_tmp, shift, end_vel_tmp, acc3, t_acc3);

    //print_all(segments);

    return result;
}

static int route_verify_and_shift(
    Segment* segments, double end_vel, double pos_offset)
{    
    int num=0;
    
    for(int i=0; i<MAX_ROUTE_SEGMENT_NUM; ++i)
    {
        if(!std::isfinite(segments[i].end_position)) // non0/0 = INF
            break;
            
        if(!std::isfinite(segments[i].t)) // 0/0 = NAN
            break;
            
        if(segments[i].t != 0.0)
        {
            segments[num] = segments[i];
            segments[num].start_position += pos_offset;
            segments[num].end_position += pos_offset;
            ++num;
        }
    }
    
    segments[num-1].magic_flags = (end_vel == 0.0)? 0x1: 0;
    
    for(int i=num; i<5; ++i)
        memset(segments + i, 0, sizeof(Segment));
        
    return num;
}

static bool check_results(const Segment* segments, int length)
{
    for(int i=0; i<length; ++i)
    {
        if(!std::isfinite(segments[i].start_position) || 
            std::isnan(segments[i].end_position) ||
            !std::isfinite(segments[i].start_vel) ||
            !std::isfinite(segments[i].acc))
            return false;
    }
    
    return true;
}

static bool route_param_pre_process(
    double& vel, double& acc, double& dec, double& end_vel)
{
    if(acc == 0.0 || dec == 0.0)
        return false;
    
    vel = fabs(vel);
    acc = fabs(acc);
    dec= fabs(dec);
    
    return true;
}

static void route_discretization(
    Segment* segments, int length, uint32_t frequency, double& prev_t_remain)
{
    double each_t_remain[MAX_ROUTE_SEGMENT_NUM + 1] = {prev_t_remain};
    
    for(int i=0; i<length; ++i)
    {
        segments[i].t -= each_t_remain[i];
        double t_freq = segments[i].t * frequency;
        
        double t_freq_ceil = ceil(t_freq);
        if(t_freq_ceil == t_freq)
            each_t_remain[i+1] = 1.0 / frequency;
        else
            each_t_remain[i+1] = (ceil(t_freq) - t_freq) / frequency;
        
        segments[i].start_position += 
            segments[i].start_vel * each_t_remain[i] + 
            segments[i].acc * each_t_remain[i] * each_t_remain[i] / 2;
            
        segments[i].start_vel += segments[i].acc * each_t_remain[i];
        segments[i].tick = (int32_t)t_freq;
    }
    
    prev_t_remain = each_t_remain[length];
}

static int tiny_segment_merge(
    Segment* segments, int length, uint32_t frequency, int32_t& new_start_tick)
{
    Segment segments_tmp[MAX_ROUTE_SEGMENT_NUM];
    int length_new = 0;
    
    if(!length)
        return 0;
        
    for(int i=0; i<length; ++i)
    {
        if(segments[i].t < 0)
        {
            if(segments[i].magic_flags)
                segments[i].tick = -1;
            else
                continue;
        }
        segments_tmp[length_new++] = segments[i];
    }
    
    for(int i=0; i<length_new; ++i)
        segments[i] = segments_tmp[i];
    
    return length_new;
}

static double cal_shift(
    double start_vel, double end_vel, double acc, double* t)
{
    if((acc == 0) && __iseq(start_vel, end_vel))
        *t = 0.0;
    else
        *t = fabs((end_vel - start_vel) / acc);
        
    return (start_vel + end_vel) * (*t) / 2;
}

static inline bool is_acc_neg(double start_vel, double end_vel)
{
    return (end_vel < start_vel);
}

}
