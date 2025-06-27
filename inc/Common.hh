#pragma once

#include<iostream>
#include<mutex>
#include<thread>
#include<assert.h>
#include<cstring>

#ifdef _WIN32
    #include<Windows.h>
    size_t getPageSize() {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return si.dwPageSize;
    }
#else
    #include<unistd.h>
    #include <sys/mman.h>
    #include <bits/wordsize.h>
#endif

#ifdef _WIN64
    #define __64bit__ 1
#else
    #if __WORDSIZE ==  64
        #define __64bit__ 1
    #endif
#endif

static const size_t FREE_LIST_NUM   = 208;              // 空闲链表长度
static const size_t MAX_BYTES       = 256 * 1024;       // 超过该值，就不从ThreadCache申请内存
static const size_t PAGE_NUM        = 129;              // PageCache中一个span最多管理128页
static const size_t PAGE_SHIFT      = 12;               // 一页4KB


void* SystemAlloc(size_t kpage);
void SystemFree(void* ptr, size_t kpage);
void*& ObjNext(void* ptr);

struct sizeClass
{
    // 线程申请size的对齐规则：整体控制在最多10%左右的内碎片浪费
	// size范围	                对齐数		        对应哈希桶下标范围
    // [1,128]				    8B 对齐      		freelist[0,16)
    // [128+1,1024]			    16B 对齐  			freelist[16,72)
    // [1024+1,8*1024]			128B 对齐  			freelist[72,128)
    // [8*1024+1,64*1024]		1024B 对齐    		freelist[128,184)
    // [64*1024+1,256*1024]	    8*1024B 对齐  		freelist[184,208)
    static size_t RoundUp(size_t size);
    static size_t _RoundUp(size_t size, size_t alignNum);
    static size_t Index(size_t size);
    static size_t _Index(size_t size, size_t align_shift);
    static size_t NumMoveSize(size_t size);
    static size_t PageMoveSize(size_t size);
};
