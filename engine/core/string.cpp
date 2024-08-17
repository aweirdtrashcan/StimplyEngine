#include "string.h"

#include "core/logger.h"
#include "platform/platform.h"

#include <cstdarg>
#include <cstdio>
#include <exception>

class StringException : public std::exception {
public:
	StringException(const char* error_message, ...) {
		__builtin_va_list va;
		va_start(va, error_message);
		char buffer[20000];
		int32_t written = vsprintf(buffer, error_message, va);
		buffer[written] = 0;
		va_end(va);

		m_What = String(buffer, written);
	}

	const char* what() const noexcept {
		return m_What.CStr();
	}
private:
	String m_What;
};

String::String() 
	:
	m_Buffer(0) {}

String::String(const char* string) {
	uint64_t stringSize = strlen(string);

	if (stringSize > 0) {
		m_Buffer.resize(stringSize + 1);
		memcpy(m_Buffer.data(), string, stringSize);
		m_Buffer[stringSize] = 0;
	}
}

String::String(const char* string, uint64_t stringSize) {
	if (stringSize > 0) {
		m_Buffer.resize(stringSize + 1);
		memcpy(m_Buffer.data(), string, stringSize);
		m_Buffer[stringSize] = 0;
	}
}

String String::Format(const char* format, ...) {
	if (!format) {
		throw StringException("Trying to format a string that's null");
	}

	String string;

	__builtin_va_list va;
	va_start(va, format);
	char buffer[20000];
	int32_t written = vsprintf(buffer, format, va);
	buffer[written] = 0;
	va_end(va);

	string.m_Buffer.resize(written + 1);
	memcpy(string.m_Buffer.data(), buffer, written);
	string[written] = 0;

	return string;
}

bool String::StringEqual(const char* string0, const char* string1) {
	return strcmp(string0, string1) == 0;
}

bool String::StringEqualI(const char* string0, const char* string1) {
#if defined(__GNUC__)
	return strcasecmp(string0, string1) == 0;
#elif defined(_MSC_VER)
	return _strcmpi(string0, string1) == 0;
#endif
}

DirectX::XMFLOAT4 String::ToFloat4(const char* source) {
	if (!source) {
		throw StringException("Trying to convert string to float but source is nullptr");
	}

	DirectX::XMFLOAT4 float4{};
	sscanf(source, "%f %f %f %f", &float4.x, &float4.y, &float4.z, &float4.w);

	return float4;
}

DirectX::XMFLOAT3 String::ToFloat3(const char* source) {
	if (!source) {
		throw StringException("Trying to convert string to float but source is nullptr");
	}
	
	DirectX::XMFLOAT3 float3{};
	sscanf(source, "%f %f %f", &float3.x, &float3.y, &float3.z);

	return float3;
}

DirectX::XMFLOAT2 String::ToFloat2(const char* source) {
	if (!source) {
		throw StringException("Trying to convert string to float but source is nullptr");
	}

	DirectX::XMFLOAT2 float2{};
	sscanf(source, "%f %f", &float2.x, &float2.y);

	return float2;
}

bool String::IsBool(const char* source) {
	if (!source) {
		throw StringException("Trying to check if string is bool but string is nullptr");
	}

	return StringEqualI(source, "true") || StringEqualI(source, "1");
}

float String::ToFloat(const char* source) {
	if (!source) {
		Logger::Warning("String::ToFloat called with a null source string");
		return MAX_FLOAT;
	}

	float float_res = 0.0f;
	sscanf(source, "%f", &float_res);

	return float_res;
}

double String::ToDouble(const char* source) {
	if (!source) {
		Logger::Warning("String::ToDouble called with a null source string");
		return MAX_DOUBLE;
	}

	double double_res = 0.0f;
	sscanf(source, "%lf", &double_res);

	return double_res;
}

uint8_t String::Tou8(const char* source) {
	if (!source) {
		Logger::Warning("String::Tou8 called with a null source string");
		return MAX_U8;
	}

	uint8_t u8_res = 0;
	sscanf(source, "%hhu", &u8_res);

	return u8_res;
}

uint16_t String::Tou16(const char* source) {
	if (!source) {
		Logger::Warning("String::Tou16 called with a null source string");
		return MAX_U16;
	}

	uint16_t u16_res = 0;
	sscanf(source, "%hu", &u16_res);

	return u16_res;
}

uint32_t String::Tou32(const char* source) {
	if (!source) {
		Logger::Warning("String::Tou32 called with a null source string");
		return MAX_U32;
	}

	uint32_t u32_res = 0;
	sscanf(source, "%u", &u32_res);

	return u32_res;
}

uint64_t String::Tou64(const char* source) {
	if (!source) {
		Logger::Warning("String::Tou64 called with a null source string");
		return MAX_U64;
	}

	uint64_t u64_res = 0;
	sscanf(source, "%llu", &u64_res);

	return u64_res;
}

int8_t String::Toi8(const char* source) {
	if (!source) {
		Logger::Warning("String::Toi8 called with a null source string");
		return MAX_I8;
	}

	int8_t i8_res = 0;
	sscanf(source, "%hhi", &i8_res);

	return i8_res;
}

int16_t String::Toi16(const char* source) {
	if (!source) {
		Logger::Warning("String::Toi16 called with a null source string");
		return MAX_I16;
	}

	int16_t i16_res = 0;
	sscanf(source, "%hi", &i16_res);

	return i16_res;
}

int32_t String::Toi32(const char* source) {
	if (!source) {
		Logger::Warning("String::Toi32 called with a null source string");
		return MAX_I32;
	}

	int32_t i32_res = 0;
	sscanf(source, "%i", &i32_res);

	return i32_res;
}

int64_t String::Toi64(const char* source) {
	if (!source) {
		Logger::Warning("String::Toi64 called with a null source string");
		return MAX_I64;
	}

	int64_t i64_res = 0;
	sscanf(source, "%lli", &i64_res);

	return i64_res;
}

void String::Append(const String& string) {
	uint64_t old_size = GetSize();
	uint64_t new_size = GetSize() + string.GetSize();

	m_Buffer.resize(new_size + 1);	
	memcpy((m_Buffer.data() + old_size), string.CStr(), string.GetSize());
	m_Buffer[new_size] = 0;
}

void String::Append(const char* string) {
	uint64_t old_size = GetSize();
	uint64_t new_size = GetSize() + strlen(string);

	m_Buffer.resize(new_size + 1);	
	memcpy((m_Buffer.data() + old_size), string, strlen(string));
	m_Buffer[new_size] = 0;
}

void String::Append(float floatingPoint) {
	char buffer[100]{};
	snprintf(buffer, sizeof(buffer), "%f", floatingPoint);

	uint64_t old_size = GetSize();
	uint64_t new_size = GetSize() + strlen(buffer);

	m_Buffer.resize(new_size + 1);	
	memcpy((m_Buffer.data() + old_size), buffer, strlen(buffer));
	m_Buffer[new_size] = 0;
}

void String::Append(bool boolean) {
	const char* buffer = boolean ? "true" : "false";

	uint64_t old_size = GetSize();
	uint64_t new_size = GetSize() + strlen(buffer);

	m_Buffer.resize(new_size + 1);	
	memcpy((m_Buffer.data() + old_size), buffer, strlen(buffer));
	m_Buffer[new_size] = 0;
}

void String::Append(char character) {
	uint64_t old_size = GetSize();

	m_Buffer.resize(GetSize() + 2);
	
	m_Buffer[old_size - 1] = character;
	m_Buffer[old_size] = 0;
}

String String::GetFileExtension() const {
	// uint8_t header[18]{};
	// list<uint8_t> image_bytes;
	// static constexpr uint8_t DeCompressed[12] = {0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    // static constexpr uint8_t IsCompressed[12] = {0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	uint64_t stringSize = GetSize();

	char* buffer = (char*)Platform::UAlloc(stringSize + 1);
	uint64_t bufferSize = 0;

	memset(buffer, 0, stringSize + 1);

	bool found = false;

	if (stringSize >= 2) {
		for (uint64_t i = stringSize - 1; i >= 0; i--) {
			buffer[bufferSize++] = m_Buffer[i];

			if (m_Buffer[i] == '.' && (bufferSize > 1)) {
				// found something.
				found = true;
				break;
			}
		}
	} else {
		Platform::UFree(buffer);
		return "";
	}

	if (!found) {
		Platform::UFree(buffer);
		return "";
	}

	// buffer will be backwards, so reverse it.
	char* reversedBuffer = (char*)Platform::UAlloc(bufferSize + 1);
	uint64_t reversedBufferIndex = bufferSize;
	
	for (uint64_t i = 0; i < bufferSize; i++) {
		reversedBuffer[reversedBufferIndex - i - 1] = buffer[i];
	}

	String formatString = reversedBuffer;

	Platform::UFree(reversedBuffer);
	Platform::UFree(buffer);

	return formatString;
}

void String::Clear() {
	m_Buffer.remove_all();
}