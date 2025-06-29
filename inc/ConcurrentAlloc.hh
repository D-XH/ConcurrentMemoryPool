#pragma once

#include"ThreadCache.hh"
#include"ObjectPool.hpp"

void* ConcurrentAlloc(size_t size);

void ConcurrentFree(void* obj);