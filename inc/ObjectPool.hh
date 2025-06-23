#pragma once

#include"Common.hh"

template<class T>
struct ObjectPool
{
public:
    T* New(){
        T* obj = nullptr;
        if(_freeList){
            // 在空闲链表中取出一块内存
            void* next = *(void**)_freeList;
            obj = (T*)_freeList;
            _freeList = next;
        }else{
            if(sizeof(T) > _remainBytes){
                // 不够用，使用系统调用申请内存
                _remainBytes = 128 * 1024;
                _memory = (char*) malloc(_remainBytes);
                if(_memory == nullptr){
                    throw std::bad_alloc();
                }
                
            }
            // 预申请内存够用，从大块内存中取出一块
            obj = (T*)_memory;
            size_t obj_size = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
            _memory += obj_size;
            _remainBytes -= obj_size;
        }

        new(obj) T;
        return obj;
    }

    void Delete(T* obj){
        obj->~T();
        *(void**)obj = _freeList;
        _freeList = obj;
    }

private:
    char* _memory = nullptr;
    void* _freeList = nullptr;
    size_t _remainBytes = 0;
public:
    std::mutex;
};