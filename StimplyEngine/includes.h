#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include "EngineTypes.inl"
#include "Keyboard.h"
#include <exception>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

#ifdef DX12
#include <wrl.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#endif

#ifdef VK
#include <vulkan/vulkan.h>
#endif

#include "utils.h"

#undef MIN
#undef min

#undef MAX
#undef max