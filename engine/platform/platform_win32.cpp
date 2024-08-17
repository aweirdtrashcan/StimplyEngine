#if defined (PLATFORM_WINDOWS)
#include "platform.h"
#include "defines.h"
#include "window/window.h"
#include "renderer/renderer_exception.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifdef MessageBox
#undef MessageBox
#endif

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#ifdef LoadLibrary
#undef LoadLibrary
#endif

struct alignas(MINIMUM_ALIGNMENT_SIZE) alloc_header {
    size_t allocation_size;
    size_t alignment;
};

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
    Logger::Warning("Shutting down platform with %zu allocated!", platform_ptr->m_TotalAllocation);
    platform_ptr = nullptr;
}

void* Platform::UAlloc(size_t size) {
    alloc_header* header = (alloc_header*)malloc(sizeof(alloc_header) + size);
    memset(header, 0, sizeof(alloc_header) + size);
    header->allocation_size = size;

    if (!platform_ptr) {
        Logger::Warning("Allocating %zu bytes before initializing platform", size);
    } else {
        platform_ptr->m_TotalAllocation += size;
        Logger::Warning("Allocating %zu bytes, total: %zu", size, platform_ptr->m_TotalAllocation);
    }

    return from_header_to_memory(header);
}

void Platform::UFree(void* memory) {
    alloc_header* header = from_memory_to_header(memory);

    if (!platform_ptr) {
        Logger::Warning("Freeing %zu bytes before initializing platform", header->allocation_size);
    } else {
        platform_ptr->m_TotalAllocation -= header->allocation_size;
        Logger::Warning("Freeing %zu bytes, total: %zu", header->allocation_size, platform_ptr->m_TotalAllocation);
    }

    free(header);
}

void* Platform::AAlloc(size_t alignment, size_t size) {
    if (alignment < MINIMUM_ALIGNMENT_SIZE) {
        Logger::Warning("Platform::aalloc: alignment size should be greater or equal to %zu bytes", MINIMUM_ALIGNMENT_SIZE);
        return nullptr;
    }

    alloc_header* header = (alloc_header*)_aligned_malloc(size + sizeof(alloc_header), alignment);
    
    if (!header) {
        int error_number = 0;
        _get_errno(&error_number);

        if (error_number == EINVAL) {
            Logger::Warning("The alignment argument was not a power of two, or was not a multiple of sizeof(void *).");
        }
        return nullptr;
    }

    memset(header, 0, size + sizeof(alloc_header));
    header->allocation_size = size;
    header->alignment = alignment;

    if (!platform_ptr) {
        Logger::Warning("Allocating %zu bytes before initializing platform", size);
    } else {
        platform_ptr->m_TotalAllocation += size;
        Logger::Warning("Allocating %zu bytes, total: %zu", size, platform_ptr->m_TotalAllocation);
    }

    return from_header_to_memory(header);
}

void Platform::AFree(void* memory) {
    alloc_header* header = from_memory_to_header(memory);

    if (!platform_ptr) {
        Logger::Warning("Freeing %zu bytes before initializing platform", header->allocation_size);
    } else {
        platform_ptr->m_TotalAllocation -= header->allocation_size;
        Logger::Warning("Freeing %zu bytes, total: %zu", header->allocation_size, platform_ptr->m_TotalAllocation);
    }

    memset(header, 0, sizeof(alloc_header) + header->allocation_size);
    _aligned_free(header);
}

void* Platform::ZeroMemory(void* memory, size_t size) {
    return memset(memory, 0, size);
}

void Platform::Log(log_level level, const char* message) {
    static constexpr WORD colors[] = { 207, 14, 10, 8 };

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(handle, colors[level]);
    printf("%s\n", message);
    SetConsoleTextAttribute(handle, 15);
}

void* Platform::LoadLibrary(const char* libraryPath) {
    char library_name[1024]{};

    DWORD path_size = 0;

    path_size = GetCurrentDirectoryA(path_size, nullptr);

    if (path_size == 0) {
        Logger::Warning("Failed to get current directory while trying to read binary in %s", libraryPath);
        return false;
    }

    list<char> path(path_size);

    if (GetCurrentDirectoryA(path_size, path.data()) == 0) {
        Logger::Warning("Failed to get current directory while trying to read binary in %s", libraryPath);
        return false;
    }

    snprintf(library_name, sizeof(library_name), "%s\\%s.dll", path.data(), libraryPath);

    void* library = LoadLibraryA(library_name);
    
    Logger::Warning("Failed to load library %s", library_name);

    return library;
}

void Platform::UnloadLibrary(void* library) {
    FreeLibrary((HMODULE)library);
}

void* Platform::LoadLibraryFunction(void* library, const std::string& functionName) {
    return GetProcAddress((HMODULE)library, functionName.c_str());
}

void* Platform::create_vulkan_surface(Window* window, void* instance) {
    VkSurfaceKHR surface = 0;

    if (SDL_Vulkan_CreateSurface((SDL_Window*)window->get_internal_handle(), (VkInstance)instance, &surface) != SDL_TRUE) {
        Logger::Fatal("Failed to create vulkan surface");
        return nullptr;
    }

    return surface;
}

binary_info Platform::ReadBinary(const char* path) {
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
        Logger::Warning("Failed to open file %s", path);
        return {};
    }

    int64_t size = 0;
    if (!GetFileSizeEx(file, (PLARGE_INTEGER)&size)) {
        Logger::Warning("Failed to get file size of %s", path);
        CloseHandle(file);
        return {};
    }

    char* buffer = (char*)Platform::UAlloc(size);

    DWORD bytes_read;

    if (!ReadFile(
        file,
        buffer,
        (DWORD)size,
        &bytes_read,
        nullptr
    )) {
        Logger::Warning("Failed to read file %s");
        CloseHandle(file);
        return {};
    }

    CloseHandle(file);

    binary_info info;
    info.binary = buffer;
    info.size = size;

    return info;
}

int64_t Platform::GetTime() {
    static int64_t performance_frequency = 0;
    
    if (!performance_frequency) {
        QueryPerformanceFrequency((PLARGE_INTEGER)&performance_frequency);
    }

    int64_t now;
    QueryPerformanceCounter((PLARGE_INTEGER)&now);

    return (now * 1000000000ui64) / performance_frequency;
}

#endif