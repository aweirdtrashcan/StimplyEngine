#pragma once

#define SZeroFree(block) { free(block); block = 0; }

template<typename T>
inline void StimplyAlloc(T** block, int count)
{
    *block = (T*)malloc(sizeof(T) * count);
    memset(*block, 0, sizeof(T) * count);
}