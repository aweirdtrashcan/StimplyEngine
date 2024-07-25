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

    static void* ualloc(size_t size);
    static void ufree(void* memory);
    static void* aalloc(size_t alignment, size_t size);
    static void afree(void* memory);
    static void* zero_memory(void* memory, size_t size);

    static void log(log_level level, const char* message);

    static void* load_library(const char* libraryPath);
    static void unload_library(void* library);
    static void* load_library_function(void* library, const std::string& functionName);

    static void* create_vulkan_surface(Window* window, void* instance);

    static binary_info read_binary(const char* path);

private:
    static inline Platform* platform_ptr = nullptr;
    size_t m_TotalAllocation = 0;
};