#include"SpanList.hh"

ObjectPool<Span> SpanList::span_pool;

SpanList::SpanList():_size(0){
    _head = SpanList::NewSpan();
    _head->next = _head;
    _head->prev = _head;
}

void SpanList::Insert(Span *pos, Span *ptr)
{
    assert(pos);
    assert(ptr);
    assert(pos->next && pos->prev);
    Span* prev = pos->prev;
    Span* next = pos;
    ptr->prev = prev;
    ptr->next = next;
    prev->next = ptr;
    next->prev = ptr;

    // ptr->prev = pos->prev;
    // ptr->next = pos;
    // pos->prev = ptr;
    // ptr->prev->next = ptr;

    _size++;
}

Span *SpanList::Erase(Span *pos)
{
    assert(pos);
    assert(pos != _head);
    assert(pos->next && pos->prev);
    Span* prev = pos->prev;
    Span* next = pos->next;
    prev->next = next;
    next->prev = prev;
    pos->prev = pos->next = nullptr;
    // pos->next->prev = pos->prev;
    // pos->prev->next = pos->next;
    // pos->next = pos->prev = pos;

    _size--;
    return pos;
}

void SpanList::PushFront(Span *ptr)
{
    assert(ptr);
    Insert(_head->next, ptr);
}

Span *SpanList::PopFront()
{
    assert(!Empty());
    return Erase(_head->next);
}

bool SpanList::Empty(){
    return _head->next == _head;
}

Span *SpanList::Begin()
{
    return _head->next;
}

Span *SpanList::End()
{
    return _head;
}

Span* SpanList::NewSpan(){
    Span* obj = nullptr;
    span_pool.mutex.lock();
    obj = span_pool.New();
    span_pool.mutex.unlock();
    return obj;
}

void SpanList::DelSpan(Span *_span)
{
    span_pool.mutex.lock();
    span_pool.Delete(_span);
    span_pool.mutex.unlock();
}
