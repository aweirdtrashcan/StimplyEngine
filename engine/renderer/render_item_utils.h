#pragma once

#include <cstdint>

struct RenderItemCreateInfo {
    uint64_t meshSize;
    void* pMeshes;
    uint64_t indicesSize;
    void* pIndices;
};