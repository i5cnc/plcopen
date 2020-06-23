/*
 * PLCTypes.hpp
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

#ifndef _URANUS_PLCTYPES_HPP_
#define _URANUS_PLCTYPES_HPP_

#include <cstdint>
#include <memory>
#include "Global.hpp"

namespace Uranus {

#pragma pack(push)
#pragma pack(4)

typedef bool        BOOL;
typedef uint8_t     BYTE;
typedef uint8_t     USINT;
typedef int8_t      SINT;
typedef uint16_t    WORD;
typedef uint16_t    UINT;
typedef int16_t     INT;
typedef uint16_t    DWORD;
typedef uint32_t    UDINT;
typedef int32_t     DINT;
typedef uint64_t    LWORD;
typedef uint64_t    ULINT;
typedef int64_t     LINT;
typedef float       REAL;
typedef double      LREAL;
typedef char*       STRING;

typedef MC_BufferMode       MC_BUFFER_MODE;
typedef MC_TransitionMode   MC_TRANSITION_MODE;
typedef MC_CoordSystem      MC_COORD_SYSTEM;
typedef MC_CircMode         MC_CIRC_MODE;
typedef MC_CircPath         MC_CIRC_PATHCHOICE;
typedef MC_Direction        MC_DIRECTION;
typedef MC_Source           MC_SOURCE;
typedef MC_ErrorCode        MC_ERRORCODE;
typedef MC_ServoErrorCode   MC_SERVOERRORCODE;

class Axis;
typedef Axis* AXIS_REF;

class AxesGroup;
typedef AxesGroup* AXES_GROUP_REF;

class CamTable;
typedef std::shared_ptr<CamTable> MC_CAM_REF;

#pragma pack(pop)

}


#endif /** _URANUS_PLCTYPES_HPP_ **/
