/*
 * FunctionBlock.hpp
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

#ifndef _URANUS_FUNCTIONBLOCK_HPP_
#define _URANUS_FUNCTIONBLOCK_HPP_

#include "Global.hpp"

namespace Uranus {
    
#pragma pack(push)
#pragma pack(4)

class FunctionBlock
{
public:
    virtual ~FunctionBlock() = default;

    virtual void onOperationActive(int32_t customId){}
    
    virtual void onOperationAborted(int32_t customId){}
    
    virtual void onOperationDone(int32_t customId){}
    
    virtual void onOperationError(MC_ErrorCode errorCode, int32_t customId){}
};

#pragma pack(pop)

}

#endif /** _URANUS_FUNCTIONBLOCK_HPP_ **/
    
