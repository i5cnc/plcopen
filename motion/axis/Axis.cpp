/*
 * Axis.cpp
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

#include "Axis.hpp"
#include "Scheduler.hpp"
#include <stdarg.h>
#include <string.h>

namespace Uranus {

Axis::Axis()
{
}

Axis::~Axis()
{
}

double Axis::frequency(void)
{
    return mSched->frequency();
}

uint32_t Axis::tick(void)
{
    return mSched->tick();
}

void Axis::vprintLog(MC_LogLevel level, const char* fmt, va_list ap)
{
    size_t size = strlen(fmt) + 32;
    char fmtAxis[size];
    snprintf(fmtAxis, size, "Axis %d: %s", axisId(), fmt);
    mSched->vprintLog(level, fmtAxis, ap);
}

int32_t Axis::axisId(void)
{
    return mAxisId;
}

}
