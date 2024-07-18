#pragma once

#include "core/logger.h"
#include <cstddef>
#include <string>

class Window;

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
    static void* zero_memory(void* memory, size_t size);

    static void log(log_level level, const char* message);

    static void* load_library(const char* libraryPath);
    static void unload_library(void* library);
    static void* load_library_function(void* library, const std::string& functionName);

    static void* create_vulkan_surface(Window* window, void* instance);

private:
    size_t m_TotalAllocation = 0;
};