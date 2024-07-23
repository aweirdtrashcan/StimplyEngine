#pragma once

#include "defines.h"
#include "platform/platform.h"
#include "containers/list.h"

#include <cstdint>

enum PipelineStage {
    PipelineTypeMVP,

    PipelineTypeMAX
};

struct mat4 {
    float x1, x2, x3, x4;
    float y1, y2, y3, y4;
    float z1, z2, z3, z4;
    float w1, w2, w3, w4;

    mat4() {
        Platform::zero_memory(this, sizeof(*this));
        x1 = 1.0f;
        y2 = 1.0f;
        z3 = 1.0f;
        w4 = 1.0f;
    }
};

struct GlobalUniformBuffer {
    mat4 projection;
    mat4 view;
};

struct ShaderBundle {
    PipelineStage pipelineType;
    const char* shaderPath;
    HANDLE pData;
    uint64_t dataSize;
};

struct RenderItemCreateInfo {
    uint64_t verticesSize;
    HANDLE pVertices;
    uint32_t verticesCount;
    uint64_t indicesSize;
    HANDLE pIndices;
    uint32_t indicesCount;
    list<ShaderBundle> shaderBundles;
    HANDLE shader;
};