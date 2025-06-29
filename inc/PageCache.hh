#pragma once

#include"Common.hh"
#include"SpanList.hh"
#include"PageMap.hpp"
#include<unordered_map>

struct PageCache
{
public:
    static PageCache* GetInstance();
public:
    Span* GetSpan(size_t kpage);
    void ReleaseSpanToPageCache(Span* _span);

    inline Span* MapPageIdToSpan(size_t page_id);
    inline void MapSetPageIdToSpan(size_t page_id, Span* _span);
    inline void MapDelByPageId(size_t page_id);

    Span* MapPtrToSpan(void* ptr);
    void MapSetPtrToSpan(void* ptr, Span* _span);
    void MapDelByPtr(void* ptr);
private:
    PageCache() = default;
    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;
public:
    std::mutex _mtx;
private:
#ifdef __64bit__
    // std::unordered_map<size_t, Span*> _idSpanMap;
    PageMap3<64 - PAGE_SHIFT> _idSpanMap;
#else
    PageMap2<32 - PAGE_SHIFT> _idSpanMap;
#endif
    SpanList _SpanLists[PAGE_NUM];
    static PageCache _sInst;
};