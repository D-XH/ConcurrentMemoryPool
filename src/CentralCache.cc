#include"CentralCache.hh"

CentralCache CentralCache::_sInst;

CentralCache *CentralCache::GetInstance()
{
    return &_sInst;
}

size_t CentralCache::FetchRangeObj(void *&start, void *&end, size_t batchNum, size_t alignSize)
{
    // 在对应桶中申请
    size_t index = sizeClass::Index(alignSize);
    // 为了线程安全，对桶进行上锁
    _SpanList[index]._mtx.lock();
    Span* span = GetOneSpan(_SpanList[index], alignSize);
    assert(span);
    assert(span->_freeList);

    size_t actual_num = 1;
    start = end = span->_freeList;
    while(ObjNext(end) != nullptr && actual_num < batchNum){
        end = ObjNext(end);
        actual_num++;
    }
    span->use_count += actual_num;
    span->_freeList = ObjNext(end);
    ObjNext(end) = nullptr;

    _SpanList[index]._mtx.unlock();
    return actual_num;
}

Span *CentralCache::GetOneSpan(SpanList& list, size_t alignSize)
{
    // 先在cc中找一下有没有管理空间非空的span
    Span* it = list.Begin();
    while(it != list.End()){
        if(it->_freeList != nullptr){
            return it;
        }
        it = it->next;
    }
    list._mtx.unlock();

    // 从pageCache获取能够空间足够的Span（pagecache中的span以页为单位）
    PageCache::GetInstance()->_mtx.lock();
    Span* _span = PageCache::GetInstance()->GetSpan(sizeClass::PageMoveSize(alignSize));
    PageCache::GetInstance()->_mtx.unlock();

    // _span没有划分成小块空间

    char* start = (char*)(_span->page_id << PAGE_SHIFT);
    char* end = start + (_span->page_num << PAGE_SHIFT);
    _span->_freeList = start;

    do{
        ObjNext(start) = start + alignSize < end ? start + alignSize : nullptr;
        start = (char*)ObjNext(start);
    }while(start != nullptr);
    _span->obj_size = alignSize;
    

    list._mtx.lock();
    list.PushFront(_span);
    return _span;
}

void CentralCache::ReleaseListToSpans(void *start, size_t index)
{
    assert(start);
    // 要访问CentralCache中临界资源SpanList，上锁
    _SpanList[index]._mtx.lock();

    while(start){
        void* next = ObjNext(start);
        Span* _span = PageCache::GetInstance()->MapPtrToSpan(start);
        ObjNext(start) = _span->_freeList;
        _span->_freeList = start;
        _span->use_count--;

        if(_span->use_count == 0){
            _SpanList[index].Erase(_span);

            // 访问PageCache中临界资源SpanList，上PageCache整体锁
            _SpanList[index]._mtx.unlock();
            PageCache::GetInstance()->_mtx.lock();
            PageCache::GetInstance()->ReleaseSpanToPageCache(_span);
            PageCache::GetInstance()->_mtx.unlock();
            _SpanList[index]._mtx.lock();
        }
        start = next;

    }
    _SpanList[index]._mtx.unlock();
    
}
