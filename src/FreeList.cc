#include"FreeList.hh"
#include "SpanList.hh"

void FreeList::Push(void* ptr)
{
    
    ObjNext(ptr) = _freeList;
    _freeList = ptr;
    _size++;
}

void FreeList::PushRange(void *start, void *end, size_t _n)
{
    ObjNext(end) = _freeList;
    _freeList = start;
    _size += _n;
}

void *FreeList::Pop()
{
    void* obj = _freeList;
    _freeList = ObjNext(_freeList);
    _size--;
    return obj;
}

void FreeList::PopRange(void *&start, void *&end, size_t _n)
{
    assert(_size >= _n);

    _size -= _n;
    start = end = _freeList;
    while(--_n){
        end = ObjNext(end);
    }
    _freeList = ObjNext(end);
    ObjNext(end) = nullptr;
}

bool FreeList::Empty()
{
    return _freeList == nullptr;
}

size_t FreeList::Size()
{
    return _size;
}

size_t& FreeList::MaxSize()
{
    return _max_size;
}
