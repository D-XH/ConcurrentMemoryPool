#include "Common.hh"

void *SystemAlloc(size_t kpage){
#ifdef _WIN32
    void* ptr = VirtualAlloc(0, kpage<<PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void* ptr = mmap(NULL, kpage<<PAGE_SHIFT, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    if(ptr == MAP_FAILED){
        perror("");
        throw std::bad_alloc();
    }
    if(ptr == nullptr){
        throw std::bad_alloc();
    }
    return ptr;
}

void SystemFree(void *ptr, size_t kpage){
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, kpage<<PAGE_SHIFT);
#endif   
}

void *& ObjNext(void *ptr){
    return  *(void**)ptr;
}

size_t sizeClass::RoundUp(size_t size)
{
    assert(size > 0);

    if(size <= 128){
        return _RoundUp(size, 8);               // 申请内存在 [1B, 128B] 内
    }else if(size <= 1024){
        return _RoundUp(size, 16);              // 申请内存在 (128B,1KB] 内
    }else if(size <= 8*1024){
        return _RoundUp(size, 128);             // 申请内存在 (1KB,8KB] 内
    }else if(size <= 64*1024){
        return _RoundUp(size, 1024);            // 申请内存在 (8KB,64KB] 内
    }else if(size <= 256*1024){
        return _RoundUp(size, 8*1024);          // 申请内存在 (64KB,256KB] 内
    }else{
        return _RoundUp(size, 1<<PAGE_SHIFT);   // 申请内存大于 256KB
    }
}

size_t sizeClass::_RoundUp(size_t size, size_t alignNum)
{
    return (size + alignNum - 1) & ~(alignNum - 1);
}

size_t sizeClass::Index(size_t size)
{
    assert(size > 0);
    assert(size <= MAX_BYTES);

    static int group_array[4] = { 16, 56, 56, 56 };

    if(size <= 128){
        return _Index(size, 3);                         // 申请内存在 [1B, 128B] 内 
    }else if(size <= 1024){
        return _Index(size - 128, 4) + 16;              // 申请内存在 (128B,1KB] 内 
    }else if(size <= 8*1024){
        return _Index(size - 1024, 7) + 72;             // 申请内存在 (1KB,8KB] 内   
    }else if(size <= 64*1024){
        return _Index(size - 8*1024, 10) + 128;         // 申请内存在 (8KB,64KB] 内
    }else if(size <= MAX_BYTES){
        return _Index(size - 64*1024, 13) + 184;        // 申请内存在 (64KB,256KB] 内
    }else{
        throw std::bad_exception();
    }
}

size_t sizeClass::_Index(size_t size, size_t align_shift)
{
    return ((size + (1<<align_shift) - 1)>>align_shift) - 1; 
}

size_t sizeClass::NumMoveSize(size_t size){
    assert(size > 0);
    // 根据单词申请小块空间大小，决定小块空间的最大申请数量
    int num = MAX_BYTES / size;
    if(num > 512){
        num = 512;
    }else if(num < 2){
        num = 2;
    }
    return num;
}

size_t sizeClass::PageMoveSize(size_t size)
{
    size_t batchNum = NumMoveSize(size);
    size_t npage = (batchNum * size)>>PAGE_SHIFT;
    return npage ? npage : 1;
}
