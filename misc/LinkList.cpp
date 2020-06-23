/*
 * LinkList.cpp
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
 
#include "LinkList.hpp"

namespace Uranus {
    
LinkNode::LinkNode()
{
    mPrev = mNext = 0;
}

LinkNode::~LinkNode()
{
    takeOut();
}
    
void LinkNode::takeOut(void)
{
    if(mNext)  mNext->mPrev = mPrev;
    if(mPrev)  mPrev->mNext = mNext;
    mNext = mPrev = 0;
}

void LinkNode::insertBack(LinkNode* one)
{
    takeOut();
    
    mNext = one->mNext;
    mPrev = one;
    
    if(mNext)  mNext->mPrev = this;
    one->mNext = this;
}

void LinkNode::insertFront(LinkNode* one)
{
    takeOut();
    
    mPrev = one->mPrev;
    mNext = one;
    
    if(mPrev)  mPrev->mNext = this;
    one->mPrev = this;
}

LinkNode* LinkNode::next(void) const
{
    return mNext;
}

LinkNode* LinkNode::prev(void) const
{
    return mPrev;
}

}
