
#include "ConcurrentAlloc.hh"

void *ConcurrentAlloc(size_t size){
    if(size > MAX_BYTES){
        size_t alignSize = sizeClass::RoundUp(size);

        PageCache::GetInstance()->_mtx.lock();
        Span* _span = PageCache::GetInstance()->GetSpan(alignSize>>PAGE_SHIFT);
        _span->obj_size = alignSize;
        PageCache::GetInstance()->_mtx.unlock();

        return (void*)(_span->page_id<<PAGE_SHIFT);
    }

    if(pTLSThreadCache == nullptr){
        static ObjectPool<ThreadCache> _obj_pool;   // 创建静态变量，所有线程共享。不用每个线程都创建一个objpool。但是要上锁
        _obj_pool.mutex.lock();
        if(pTLSThreadCache == nullptr){
            pTLSThreadCache = _obj_pool.New();  
        }
        _obj_pool.mutex.unlock();
    }
    
    return pTLSThreadCache->Allocate(size);
    
}

void ConcurrentFree(void *obj){
    assert(obj);

    Span* _span = PageCache::GetInstance()->MapPtrToSpan(obj);

    if(_span->obj_size <= MAX_BYTES){
        pTLSThreadCache->Deallocate(obj, _span->obj_size);
    }else{
        PageCache::GetInstance()->_mtx.lock();
        PageCache::GetInstance()->ReleaseSpanToPageCache(_span);
        PageCache::GetInstance()->_mtx.unlock();
    }
}