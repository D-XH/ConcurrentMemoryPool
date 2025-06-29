#include"PageCache.hh"

PageCache PageCache::_sInst;

PageCache* PageCache::GetInstance(){
    return &_sInst;
}

Span* PageCache::GetSpan(size_t kpage){

    if(kpage > PAGE_NUM - 1){
        Span* _span = SpanList::NewSpan();
        _span->page_id = (size_t)SystemAlloc(kpage)>>PAGE_SHIFT;
        _span->page_num = kpage;
        _span->is_used = true;

        MapSetPageIdToSpan(_span->page_id, _span);
        return _span;
    }

    // 小于等于128时，从保存k页的spanlist桶开始，找到空闲的span。
    // 没有就到更多页的spanlist桶中找，直到找完保存128页的桶
    // 都没有则向OS申请内存

    // 1. k号桶有span
    if(!_SpanLists[kpage].Empty()){
        Span* _span = _SpanLists[kpage].PopFront();
        _span->is_used = true;

        for(int i = 0; i < kpage; ++i){
            MapSetPageIdToSpan(_span->page_id + i, _span);
        }
        return _span;
    }

    // 2. k号桶没有，后面桶有
    for(size_t i = kpage+1; i < PAGE_NUM; ++i){
        if(!_SpanLists[i].Empty()){
            // 获取桶中的Span
            Span* ret_span = _SpanLists[i].PopFront();
            // 创建新的Span，把原Spa内进行划分
            Span* tmp_span = SpanList::NewSpan();

            tmp_span->page_id = ret_span->page_id + kpage;
            tmp_span->page_num = i - kpage;
            ret_span->page_num = kpage;
            ret_span->is_used = true;

            _SpanLists[tmp_span->page_num].PushFront(tmp_span);
            MapSetPageIdToSpan(tmp_span->page_id, tmp_span);
            MapSetPageIdToSpan(tmp_span->page_id + tmp_span->page_num - 1, tmp_span);

            for(int i = 0; i < kpage; ++i){
                MapSetPageIdToSpan(ret_span->page_id + i, ret_span);
            }
            return ret_span;
        }
    }

    // 3. 都没有
    Span* tmp_span = SpanList::NewSpan();
    tmp_span->page_id = (size_t)SystemAlloc(PAGE_NUM - 1) >> PAGE_SHIFT;
    tmp_span->page_num = PAGE_NUM - 1;

    _SpanLists[tmp_span->page_num].PushFront(tmp_span);
    return GetSpan(kpage);
}

void PageCache::ReleaseSpanToPageCache(Span *_span)
{
    assert(_span);
    _span->is_used = false;
    _span->_freeList = nullptr;
    _span->prev = _span->next = nullptr;

    if(_span->page_num > PAGE_NUM - 1){
        SystemFree((void*)(_span->page_id<<PAGE_SHIFT), _span->page_num);
        SpanList::DelSpan(_span);

        return;
    }

    // 对自己
    for(int i = 0; i < _span->page_num; ++i){
        MapDelByPageId(_span->page_id + i);
    }

    // 对相邻页进行合并
    size_t left = _span->page_id - 1;
    size_t right = _span->page_id + _span->page_num;
    // 1. 向左
    Span* it_span = MapPageIdToSpan(left);
    while(it_span != nullptr){
        if(it_span->is_used || (it_span->page_num + _span->page_num > PAGE_NUM - 1)){
            break;
        }
        _span->page_id = it_span->page_id;
        _span->page_num += it_span->page_num;
        
        MapDelByPageId(left);
        if(it_span->page_num > 1){
            MapDelByPageId(it_span->page_id);
        }
        left = it_span->page_id - 1;

        _SpanLists[it_span->page_num].Erase(it_span);
        SpanList::DelSpan(it_span);
        it_span = MapPageIdToSpan(left);
    }
    // 2. 向右
    it_span = MapPageIdToSpan(right);
    while(it_span != nullptr){
        // 
        if(it_span->is_used || (it_span->page_num + _span->page_num > PAGE_NUM - 1)){
            break;
        }
        _span->page_num += it_span->page_num;

        MapDelByPageId(right);
        if(it_span->page_num > 1){
            MapDelByPageId(right + it_span->page_num - 1);
        }
        right += it_span->page_num;

        _SpanLists[it_span->page_num].Erase(it_span);
        SpanList::DelSpan(it_span);
        it_span = MapPageIdToSpan(right);
    } 

    _SpanLists[_span->page_num].PushFront(_span);
    MapSetPageIdToSpan(_span->page_id, _span);
    MapSetPageIdToSpan(_span->page_id + _span->page_num - 1, _span);
}

Span *PageCache::MapPageIdToSpan(size_t page_id)
{
    // Span* _span = _idSpanMap[page_id];  // unorder_map
    Span* _span = (Span*)_idSpanMap.Get(page_id);
    return _span;
}

Span *PageCache::MapPtrToSpan(void *ptr)
{
    size_t _page_id = (size_t)ptr>>PAGE_SHIFT;

    // std::unique_lock<std::mutex> lc(_mtx);
    return MapPageIdToSpan(_page_id);
}

void PageCache::MapSetPageIdToSpan(size_t page_id, Span *_span)
{
    // _idSpanMap[page_id] = _span;    // unorder_map
    _idSpanMap.Set(page_id, _span);
}

void PageCache::MapSetPtrToSpan(void *ptr, Span *_span)
{
    MapSetPageIdToSpan((size_t)ptr>>PAGE_SHIFT, _span);
}

void PageCache::MapDelByPtr(void *ptr)
{
    MapDelByPageId((size_t)ptr>>PAGE_SHIFT);
}

void PageCache::MapDelByPageId(size_t page_id)
{
    // _idSpanMap.erase(page_id);  // unorder_map
    _idSpanMap.Del(page_id);
}
