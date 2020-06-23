/*
 * LinkList.hpp
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
 
#ifndef _URANUS_LINKLIST_HPP_
#define _URANUS_LINKLIST_HPP_

namespace Uranus {

class LinkNode
{
private:
    LinkNode* mPrev;
    LinkNode* mNext;
    
public:
    LinkNode();
    virtual ~LinkNode();
    
    void takeOut(void);
    void insertBack(LinkNode* one);
    void insertFront(LinkNode* one);
    LinkNode* next(void) const;
    LinkNode* prev(void) const;
};

}

#endif /** _URANUS_LINKLIST_HPP_ **/
