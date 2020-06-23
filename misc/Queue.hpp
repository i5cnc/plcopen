/*
 * Queue.hpp
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
 
#ifndef _URANUS_QUEUE_HPP_
#define _URANUS_QUEUE_HPP_

template <typename T, size_t Size>
class Queue
{
private:
    size_t mHead = 0;
    size_t mTail = 0;
    size_t mUsed = 0;
    size_t mSize;
    T queue[Size+1];
    
public:
    Queue();
    ~Queue();
    
    T* front(void) const;
    T* back(void) const;
    
    T* next(T*) const;
    T* prev(T*) const;
    
    bool pop_front(void);
    bool pop_back(void);
    
    bool push_back(void);
    
    void clear(void);
    bool empty(void) const;
    size_t used(void) const;
};

#define INC(num, limit) \
    if(num >= limit - 1) num = 0; \
    else ++num;
    
#define DEC(num, limit) \
    if(num <= 0) num = limit - 1; \
    else --num;

template <typename T, size_t Size>
inline Queue<T, Size>::Queue()
{
    mSize = Size + 1;
}

template <typename T, size_t Size>
inline Queue<T, Size>::~Queue()
{
}

template <typename T, size_t Size>
inline T* Queue<T, Size>::front(void) const
{
    if(mHead == mTail)
        return 0;
        
    T* tmp = (T*)queue + mHead;
    return tmp;
}

template <typename T, size_t Size>
inline T* Queue<T, Size>::back(void) const
{
    if(mHead == mTail)
        return 0;
            
    size_t pos = mTail;
    DEC(pos, mSize);
    T* tmp = (T*)queue + pos;
    return tmp;
}

template <typename T, size_t Size>
inline T* Queue<T, Size>::next(T* one) const
{
    size_t cur = one - queue;
    INC(cur, mSize);
    
    if((mHead < mTail && cur >= mHead && cur < mTail) ||
        (mTail < mHead && (cur >= mHead || cur < mTail)))
        return (T*)queue + cur;
    else
        return 0;
}

template <typename T, size_t Size>
inline T* Queue<T, Size>::prev(T* one) const
{
    size_t cur = one - queue;
    DEC(cur, mSize);
    
    if((mHead < mTail && cur >= mHead && cur < mTail) ||
        (mTail < mHead && (cur >= mHead || cur < mTail)))
        return (T*)queue + cur;
    else
        return 0;
}

template <typename T, size_t Size>
inline bool Queue<T, Size>::pop_front(void)
{
    if(mHead == mTail)
        return false;
        
    INC(mHead, mSize);
    --mUsed;
    return true;
}

template <typename T, size_t Size>
bool Queue<T, Size>::pop_back(void)
{
    if(mHead == mTail)
        return false;
        
    DEC(mTail, mSize);
    --mUsed;
    return true;
}

template <typename T, size_t Size>
inline bool Queue<T, Size>::push_back(void)
{
    if((mTail == (mHead - 1)) || ((mTail - mSize) == (mHead - 1)))
        return false;
    
    INC(mTail, mSize);
    ++mUsed;
    return true;
}

template <typename T, size_t Size>
inline void Queue<T, Size>::clear(void)
{
    mHead = mTail;
}

template <typename T, size_t Size>
inline bool Queue<T, Size>::empty(void) const
{
    bool result = (mTail == mHead);
    return result;
}

template <typename T, size_t Size>
size_t Queue<T, Size>::used(void) const
{
    return mUsed;
}

#undef INC
#undef DEC

#endif /** _URANUS_QUEUE_HPP_ **/
