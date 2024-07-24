#pragma once

#include <DirectXMath.h>

struct GlobalUniformObject {
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX reserved[2];
};