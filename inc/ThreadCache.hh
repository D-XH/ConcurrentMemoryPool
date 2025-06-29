#pragma once

#include"Common.hh"
#include"FreeList.hh"
#include"CentralCache.hh"

struct ThreadCache
{
public:
    void* Allocate(size_t size);
    void Deallocate(void* obj, size_t size);
    void* FetchFromCentralCache(size_t index, size_t size);
    void ListToLoog(FreeList& list, size_t index);
private:
    FreeList _freeLists[FREE_LIST_NUM];
};

static __thread ThreadCache* pTLSThreadCache = nullptr;