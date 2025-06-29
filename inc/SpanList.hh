#pragma once

#include"Common.hh"
#include"FreeList.hh"
#include"ObjectPool.hpp"

struct Span
{
public:
    Span* next = nullptr;
    Span* prev = nullptr;
public:
    size_t page_id = 0;
    size_t page_num = 0;
    size_t obj_size = 0;
    size_t use_count = 0;
    bool is_used = false;
    void* _freeList = nullptr;

};

struct SpanList
{
public:
    SpanList();
public:
    void Insert(Span* pos, Span* ptr);
    void PushFront(Span* ptr);
    Span* Erase(Span* pos);
    Span* PopFront();
    Span* Begin();
    Span* End();
    bool Empty();
public:
    size_t _size = 0;
    std::mutex _mtx;
    Span* _head = nullptr;
public:
    static Span* NewSpan();
    static void DelSpan(Span* _span);
private:
    static ObjectPool<Span> span_pool;
};