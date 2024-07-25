#pragma once

#include "core/logger.h"
#include <cstddef>
#include <cstdint>
#include <string>

class Window;

struct binary_info {
    uint64_t size;
    char* binary;
};

static inline constexpr uint64_t MINIMUM_ALIGNMENT_SIZE = 16;

class RAPI Platform {
public:
    Platform();
    Platform(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(const Platform&) = delete;
    ~Platform();

    static Platform* Get();

    static void* UAlloc(size_t size);
    static void UFree(void* memory);
    static void* AAlloc(size_t alignment, size_t size);
    static void AFree(void* memory);
    static void* ZeroMemory(void* memory, size_t size);

    static void log(log_level level, const char* message);

    static void* LoadLibrary(const char* libraryPath);
    static void UnloadLibrary(void* library);
    static void* LoadLibraryFunction(void* library, const std::string& functionName);

    static void* create_vulkan_surface(Window* window, void* instance);

    [[nodiscard("binary_info contains a dynamically allocated string")]] 
    static binary_info ReadBinary(const char* path);

    /* Returns the current time in nanoseconds */
    static int64_t GetTime();

private:
    static inline Platform* platform_ptr = nullptr;
    size_t m_TotalAllocation = 0;
};