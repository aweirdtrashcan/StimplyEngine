#pragma once

#include <DirectXMath.h>

struct global_uniform_object {
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX reserved[2];
};