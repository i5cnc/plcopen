/*
 * AxisStatus.hpp
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

#ifndef _URANUS_AXISSTATUS_HPP_
#define _URANUS_AXISSTATUS_HPP_

#include "AxisBase.hpp"

namespace Uranus {
    
class AxisStatus : virtual public AxisBase
{
public:
    AxisStatus();
    virtual ~AxisStatus();
    
    MC_AxisStatus status(void);
    MC_ErrorCode setStatus(MC_AxisStatus status);
    MC_ErrorCode testStatus(MC_AxisStatus status);
    
private:
    static void onErrorHandler(AxisBase* this_, MC_ErrorCode errorCode);
    static void onPowerStatusChangedHandler(AxisBase* this_, bool powerStatus);
    
private:
    class AxisStatusImpl;
    AxisStatusImpl* mImpl_;
};

}

#endif /** _URANUS_AXISSTATUS_HPP_ **/
    
