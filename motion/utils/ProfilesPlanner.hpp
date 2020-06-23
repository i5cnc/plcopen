/*
 * ProfilesPlanner.hpp
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
 
#ifndef _URANUS_PROFILESPLANNER_HPP_
#define _URANUS_PROFILESPLANNER_HPP_

#include "ExeclQueue.hpp"
#include "ProfilePlanner.hpp"

namespace Uranus {

class ProfileNode : virtual public ExeclNode
{
public:
    double mStartPos = 0;
    double mStartVel = 0;
    double mStartAcc = 0; //保留
    
    double mEndPos = 0;
    double mEndVel = 0;
    double mEndAcc = 0; //保留
    
    double mVel = 0;
    double mAcc = 0;
    double mDec = 0;
    double mJerk = 0;
    
    friend class ProfilesPlanner;
};

class ProfilesPlanner : public ProfilePlanner
{
public:
    ProfilesPlanner();
    virtual ~ProfilesPlanner();

    bool plan(
        ProfileNode* node, 
        double startPos, 
        double startVel, 
        double startAcc);
    
private:
    class ProfilesPlannerImpl;
    ProfilesPlannerImpl* mImpl_;
};

}

#endif /** _URANUS_PROFILESPLANNER_HPP_ **/
