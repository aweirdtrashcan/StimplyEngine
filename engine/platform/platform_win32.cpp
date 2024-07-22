#include "platform.h"

#if defined (PLATFORM_WINDOWS)
#include "defines.h"
#include "window/window.h"
#include "renderer/renderer_exception.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifdef MessageBox
#undef MessageBox
#endif

struct alloc_header {
    size_t allocation_size;
};

static Platform* platform_ptr = nullptr;

void Window::MessageBox(const char* title, const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, nullptr);
}

static inline void* from_header_to_memory(alloc_header* header) {
    return ((uint8_t*)header) + sizeof(alloc_header);
}

static inline alloc_header* from_memory_to_header(void* memory) {
    return (alloc_header*)(((uint8_t*)memory) - sizeof(alloc_header));
}

Platform::Platform() {
    if (platform_ptr) {
        // TODO: Throw exception
    }
    platform_ptr = this;
}

Platform::~Platform() {
    platform_ptr = nullptr;
}

void* Platform::ualloc(size_t size) {
    alloc_header* header = (alloc_header*)malloc(sizeof(alloc_header) + size);
    memset(header, 0, sizeof(alloc_header) + size);
    header->allocation_size = size;

    if (!platform_ptr) {
        Logger::warning("Allocating %zu bytes before initializing platform", size);
        platform_ptr->m_TotalAllocation += size;
    }

    return from_header_to_memory(header);
}

void Platform::ufree(void* memory) {
    alloc_header* header = from_memory_to_header(memory);

    if (!platform_ptr) {
        Logger::warning("Freeing %zu bytes before initializing platform", header->allocation_size);
        platform_ptr->m_TotalAllocation -= header->allocation_size;
    }

    free(header);
}

void* Platform::zero_memory(void* memory, size_t size) {
    return memset(memory, 0, size);
}

void Platform::log(log_level level, const char* message) {
    static constexpr WORD colors[] = { 207, 14, 10, 8 };

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(handle, colors[level]);
    printf("%s\n", message);
    SetConsoleTextAttribute(handle, 15);
}

void* Platform::load_library(const char* libraryPath) {
    char library_name[1024]{};

    throw RendererException("Configure current path");
    
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '/') {
            path[i] = '\\';
        }
    }

    snprintf(library_name, sizeof(library_name), "%s\\%s.dll", path.c_str(), libraryPath);

    void* library = LoadLibraryA(library_name);
    
    // TODO: Error message

    return library;
}

void Platform::unload_library(void* library) {
    FreeLibrary((HMODULE)library);
}

void* Platform::load_library_function(void* library, const std::string& functionName) {
    return GetProcAddress((HMODULE)library, functionName.c_str());
}

void* Platform::create_vulkan_surface(Window* window, void* instance) {
    VkSurfaceKHR surface = 0;

    if (SDL_Vulkan_CreateSurface((SDL_Window*)window->get_internal_handle(), (VkInstance)instance, &surface) != SDL_TRUE) {
        Logger::fatal("Failed to create vulkan surface");
        return nullptr;
    }

    return surface;
}

binary_info Platform::read_binary(const char* path) {
    HANDLE file = CreateFileA(
        path,
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY,
        nullptr
    );

    if (!file) {
        Logger::debug("Failed to open file %s", path);
        return {};
    }

    int64_t size = 0;
    if (!GetFileSizeEx(file, (PLARGE_INTEGER)&size)) {
        Logger::debug("Failed to get file size of %s", path);
        CloseHandle(file);
        return {};
    }

    char* buffer = (char*)Platform::ualloc(size);

    DWORD bytes_read;

    if (!ReadFile(
        file,
        buffer,
        size,
        &bytes_read,
        nullptr
    )) {
        Logger::debug("Failed to read file %s");
        CloseHandle(file);
        return {};
    }

    CloseHandle(file);

    binary_info info;
    info.binary = buffer;
    info.size = size;

    return info;
}

#endif