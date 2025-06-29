#pragma once

#include"Common.hh"

struct FreeList
{
public:
    void Push(void* ptr);
    void PushRange(void* start, void* end, size_t _n);
    void* Pop();
    void PopRange(void*& start, void*& end, size_t _n);
    bool Empty();
    size_t Size();
    size_t& MaxSize();
private:
    size_t _size = 0;
    size_t _max_size = 1;
    void* _freeList = nullptr;
};