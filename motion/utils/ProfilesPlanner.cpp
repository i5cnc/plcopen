/*
 * ProfilesPlanner.cpp
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
 
#include "ProfilesPlanner.hpp"
#include "ProfilePlanner.hpp"
#include "MathUtils.hpp"

namespace Uranus {
    
class ProfilesPlanner::ProfilesPlannerImpl
{
public:
    int mPlanRet = -1;
    double mStartPos = 0;
    double mStartVel = 0;
    double mStartAcc = 0; //reserved
};
    
ProfilesPlanner::ProfilesPlanner()
{
    mImpl_ = new ProfilesPlannerImpl();
}

ProfilesPlanner::~ProfilesPlanner()
{
    delete mImpl_;
}

bool ProfilesPlanner::plan(
    ProfileNode* node, double startPos, double startVel, double startAcc)
{
    return ProfilePlanner::plan(
        startPos, node->mEndPos, 
        startVel, node->mVel, node->mEndVel, 
        node->mAcc, node->mDec);
}

};
