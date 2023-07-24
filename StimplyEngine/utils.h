#pragma once

template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
	return val < min ? min : val > max ? max : val;
}

inline const char* TranslateDX11Error(long hres)
{
	switch (hres)
	{
	case (long)0x887C0002:
		return "D3D11_ERROR_FILE_NOT_FOUND";
	case (long)0x887C0001:
		return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
	case (long)0x80004005:
		return "E_FAIL";
	case (long)0x80070057:
		return "E_INVALIDARG";
	case (long)0x8007000E:
		return "E_OUTOFMEMORY";
	case 1L:
		return "S_FALSE";
	default:
		return "Unknown";
	}
}