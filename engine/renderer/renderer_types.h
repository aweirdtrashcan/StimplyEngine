#pragma once

#include "defines.h"
#include <DirectXMath.h>

enum PipelineStage {
    PipelineTypeMVP,

    PipelineTypeMAX
};

struct GlobalUniformBuffer {
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;
};

struct RenderItemCreateInfo {
    uint64_t vertexSize;
    HANDLE pVertices;
    uint32_t verticesCount;
    uint64_t indexSize;
    HANDLE pIndices;
    uint32_t indicesCount;
    HANDLE texture;
};

enum FrameStatus {
    FRAME_STATUS_FAILED,
    FRAME_STATUS_SUCCESS,
    FRAME_STATUS_SKIP
};

struct GlobalUniformObject {
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX view;
};

struct LocalUniformObject {
    DirectX::XMFLOAT4 diffuseColor;
};
