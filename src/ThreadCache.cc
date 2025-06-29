#include "ThreadCache.hh"

void *ThreadCache::Allocate(size_t size)
{
    assert(size <= MAX_BYTES);
    size_t alignSize = sizeClass::RoundUp(size);
    size_t index = sizeClass::Index(size);

    if(!_freeLists[index].Empty()){
        return _freeLists[index].Pop();
    }else{
        return FetchFromCentralCache(index, alignSize);
    }
}

void ThreadCache::Deallocate(void* obj, size_t size)
{
    assert(obj);
    assert(size <= MAX_BYTES);
    // 还给threadCache
    size_t index = sizeClass::Index(size);
    _freeLists[index].Push(obj);
    // 还给centralCache
    if(_freeLists[index].Size() >= _freeLists[index].MaxSize()){
        ListToLoog(_freeLists[index], index);
    }
    

}

void *ThreadCache::FetchFromCentralCache(size_t index, size_t alignSize)
{
    // 在对应桶中
    // 根据慢开始算法，得到单次申请的空间数量
    size_t batchNum = std::min(sizeClass::NumMoveSize(alignSize), _freeLists[index].MaxSize());

    if(batchNum == _freeLists[index].MaxSize()){
        _freeLists[index].MaxSize()++;
    }

    void* start = nullptr;
    void* end = nullptr;

    size_t actual_num = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, alignSize);
    
    assert(actual_num >= 1);
    if(actual_num > 1){
        _freeLists[index].PushRange(ObjNext(start), end, actual_num - 1);
    }
    return start;
}

void ThreadCache::ListToLoog(FreeList& list, size_t index)
{
    void* start = nullptr;
    void* end = nullptr;
    list.PopRange(start, end, list.MaxSize());

    CentralCache::GetInstance()->ReleaseListToSpans(start, index);
}
