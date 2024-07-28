#pragma once

#include <cstdint>

typedef void* HANDLE;
static inline constexpr unsigned int INVALID_ID = 0xffffffff;

#if defined(__GNUC__)
#define string_cmpi_length(str0, str1, length) (strncasecmp(str0, str1, length) == 0);
#define string_cmpi(str0, str1) (strcasecmp(str0, str1) == 0);
#define string_append_string(dest, source, append)
#define AINLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define string_cmpi_length(str0, str1, length) (_strnicmp(str0, str1, length) == 0);
#define string_cmpi(str0, str1) (_strcmpi(str0, str1) == 0);
#define AINLINE __force_inline
#endif

static constexpr inline double MAX_DOUBLE = 1.7976931348623157e+308;
static constexpr inline float MAX_FLOAT = 3.40282347e+38F;

static constexpr inline uint8_t MAX_U8 = 0xff;
static constexpr inline int8_t MAX_I8 = 0x7f;

static constexpr inline uint16_t MAX_U16 = 0xffff;
static constexpr inline int16_t MAX_I16 = 0x7fff;

static constexpr inline uint32_t MAX_U32 = 0xffffffff;
static constexpr inline int32_t MAX_I32 = 0x7fffffff;

static constexpr inline uint64_t MAX_U64 = 0xffffffffffffffff;
static constexpr inline int64_t MAX_I64 = 0x7fffffffffffffff;