
#ifndef __OBJECT_POOL_HPP__
#define __OBJECT_POOL_HPP__ 1

#include"Common.hh"

template<class T, size_t SIZE = 128 * 1024>
struct ObjectPool
{
public:
    ObjectPool(){}
    ~ObjectPool(){

    }
public:
    T* New(){
        T* obj = nullptr;
        if(_freeList){
            // 在空闲链表中取出一块内存
            void* next = *(void**)_freeList;
            obj = (T*)_freeList;
            _freeList = next;
            // _freeBytes -= sizeof(T);
        }else{
            if(sizeof(T) > _remainBytes){
                // 不够用，使用系统调用申请内存
                _remainBytes = SIZE;
                size_t alloc_page = _remainBytes>>PAGE_SHIFT;
                alloc_page = alloc_page? alloc_page : 1;
                _memory = (char*) SystemAlloc(alloc_page);
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
        // _freeBytes += sizeof(T);
        // if(_freeBytes > 16*SIZE){
        //     SystemFree(_freeList, _freeBytes>>PAGE_SHIFT);
        //     _freeBytes = 0;
        // }
    }

private:
    char* _memory = nullptr;
    void* _freeList = nullptr;
    size_t _remainBytes = 0;
    // size_t _freeBytes = 0;
public:
    std::mutex mutex;
};

#endif