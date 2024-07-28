#pragma once

#include "defines.h"
#include "containers/list.h"

#include <DirectXMath.h>

class String {
public:
	String();
	String(const char* string);
	String(const char* string, uint64_t stringSize);

	static String Format(const char* format, ...);  

	AINLINE operator const char*() {
		return m_Buffer.data();
	}

	AINLINE uint64_t GetSize() const { return m_Buffer.size(); }

	static bool StringEqual(const char* string0, const char* string1);
	static bool StringEqualI(const char* string0, const char* string1);
	
	AINLINE static bool StringNotEqual(const char* string0, const char* string1) { 
		return !String::StringEqual(string0, string1); 
	}
	
	AINLINE static bool StringNotEqualI(const char* string0, const char* string1) { 
		return !String::StringEqualI(string0, string1); 
	}

	friend bool operator==(const String& string0, const String& string1) {
		return String::StringEqual(string0.CStr(), string1.CStr());
	}

	friend bool operator!=(const String& string0, const String& string1) {
		return String::StringNotEqual(string0.CStr(), string1.CStr());
	}

	AINLINE const char* CStr() const { return m_Buffer.data(); }

	static DirectX::XMFLOAT4 ToFloat4(const char* source);
	static DirectX::XMFLOAT3 ToFloat3(const char* source);
	static DirectX::XMFLOAT2 ToFloat2(const char* source);

	static bool IsBool(const char* source);
	
	static float ToFloat(const char* source);
	static double ToDouble(const char* source);

	static uint8_t Tou8(const char* source);
	static uint16_t Tou16(const char* source);
	static uint32_t Tou32(const char* source);
	static uint64_t Tou64(const char* source);

	static int8_t Toi8(const char* source);
	static int16_t Toi16(const char* source);
	static int32_t Toi32(const char* source);
	static int64_t Toi64(const char* source);

	void Append(const String& string);
	void Append(const char* string);
	void Append(float floatingPoint);
	void Append(bool boolean);
	void Append(char character);

	void Clear();

	AINLINE bool IsEmpty() const { return !m_Buffer.size(); }

private:
	list<char> m_Buffer;
};