#pragma once


#include <cassert>
#include <vector>
#include <array>
#include <stdio.h>

#undef min
#undef max

//#include <backends/imgui_impl_dx11.h>
//#include <backends/imgui_impl_win32.h>
//#include <imgui.h>

#include <chrono>
#include <algorithm>

#ifdef _DEBUG
#define ThrowIfFailed(hr)\
{\
	if (FAILED(hr))\
	{\
		throw std::exception();\
	}\
}
#else
#define ThrowIfFailed(x) (x)
#endif