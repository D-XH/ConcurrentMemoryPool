#pragma once

#include"Common.hh"
#include"SpanList.hh"
#include"PageCache.hh"

struct CentralCache
{
public:
    static CentralCache* GetInstance();
    size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignSize);
    Span* GetOneSpan(SpanList& list, size_t size);
    void ReleaseListToSpans(void* start, size_t index);
private:
    CentralCache() = default;
    CentralCache(const CentralCache&) = delete;
    CentralCache& operator=(const CentralCache&) = delete;
private:
    static CentralCache _sInst;
    SpanList _SpanList[FREE_LIST_NUM];
};
