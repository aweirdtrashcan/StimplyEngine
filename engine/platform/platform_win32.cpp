#if defined (PLATFORM_WINDOWS)
#include "platform.h"
#include "defines.h"
#include "window/window.h"
#include "renderer/renderer_exception.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifdef MessageBox
#undef MessageBox
#endif

struct alignas(MINIMUM_ALIGNMENT_SIZE) alloc_header {
    size_t allocation_size;
    size_t alignment;
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
    Logger::warning("Shutting down platform with %zu allocated!", platform_ptr->m_TotalAllocation);
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

void* Platform::aalloc(size_t alignment, size_t size) {
    if (alignment < MINIMUM_ALIGNMENT_SIZE) {
        Logger::warning("Platform::aalloc: alignment size should be greater or equal to %zu bytes", MINIMUM_ALIGNMENT_SIZE);
        return nullptr;
    }

    alloc_header* header = (alloc_header*)_aligned_malloc(size + sizeof(alloc_header), alignment);
    
    if (!header) {
        int error_number = 0;
        _get_errno(&error_number);

        if (error_number == EINVAL) {
            Logger::warning("The alignment argument was not a power of two, or was not a multiple of sizeof(void *).");
        }
        return nullptr;
    }

    memset(header, 0, size + sizeof(alloc_header));
    header->allocation_size = size;
    header->alignment = alignment;

    if (!platform_ptr) {
        Logger::warning("Allocating %zu bytes before initializing platform", size);
    }
    else {
        platform_ptr->m_TotalAllocation += size;
    }

    return from_header_to_memory(header);
}

void Platform::afree(void* memory) {
    alloc_header* header = from_memory_to_header(memory);

    if (!platform_ptr) {
        Logger::warning("Freeing %zu bytes before initializing platform", header->allocation_size);
    }
    else {
        platform_ptr->m_TotalAllocation -= header->allocation_size;
    }

    memset(header, 0, sizeof(alloc_header) + header->allocation_size);
    _aligned_free(header);
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

    DWORD path_size = 0;

    path_size = GetCurrentDirectoryA(path_size, nullptr);

    if (path_size == 0) {
        Logger::warning("Failed to get current directory while trying to read binary in %s", libraryPath);
        return false;
    }

    list<char> path(path_size);

    if (GetCurrentDirectoryA(path_size, path.data()) == 0) {
        Logger::warning("Failed to get current directory while trying to read binary in %s", libraryPath);
        return false;
    }

    snprintf(library_name, sizeof(library_name), "%s\\%s.dll", path.data(), libraryPath);

    void* library = LoadLibraryA(library_name);
    
    Logger::warning("Failed to load library %s", library_name);

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
        Logger::warning("Failed to open file %s", path);
        return {};
    }

    int64_t size = 0;
    if (!GetFileSizeEx(file, (PLARGE_INTEGER)&size)) {
        Logger::warning("Failed to get file size of %s", path);
        CloseHandle(file);
        return {};
    }

    char* buffer = (char*)Platform::ualloc(size);

    DWORD bytes_read;

    if (!ReadFile(
        file,
        buffer,
        (DWORD)size,
        &bytes_read,
        nullptr
    )) {
        Logger::warning("Failed to read file %s");
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