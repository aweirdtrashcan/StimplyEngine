
#include <cstdint>
#include <ctime>
#if defined (PLATFORM_LINUX)
#include "platform.h"
#include "window/window.h"
#include "core/logger.h"

#include <SDL2/SDL_messagebox.h>
#include <SDL2/SDL_vulkan.h>

#include <dlfcn.h>
#include <unistd.h>
#include <linux/limits.h> // NOTE: I think you must have linux-headers installed, but i still need to look for that up.
#include <cerrno>
#include <cstdlib>
#include <time.h>

void Window::MessageBox(const char* title, const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, nullptr);
}

struct alignas(MINIMUM_ALIGNMENT_SIZE) alloc_header {
    size_t allocation_size;
    size_t alignment;
};

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

void* Platform::UAlloc(size_t size) {
    alloc_header* header = (alloc_header*)malloc(sizeof(alloc_header) + size);
    memset(header, 0, sizeof(alloc_header) + size);
    header->allocation_size = size;

    if (!platform_ptr) {
        Logger::warning("Allocating %zu bytes before initializing platform", size);
    } else {
        Logger::warning("Allocating %zu bytes, total: %zu", size, platform_ptr->m_TotalAllocation);    
        platform_ptr->m_TotalAllocation += size;
    }
    
    return from_header_to_memory(header);    
}

void Platform::UFree(void* memory) {
    alloc_header* header = from_memory_to_header(memory);

    if (!platform_ptr) {
        Logger::warning("Freeing %zu bytes before initializing platform", header->allocation_size);
    } else {
        Logger::warning("Freeing %zu bytes, total: %zu", header->allocation_size, platform_ptr->m_TotalAllocation);
        platform_ptr->m_TotalAllocation -= header->allocation_size;
    }

    memset(header, 0, sizeof(alloc_header) + header->allocation_size);
    free(header);
}

void* Platform::AAlloc(size_t alignment, size_t size) {
    if (alignment < MINIMUM_ALIGNMENT_SIZE) {
        Logger::warning("Platform::aalloc: alignment size should be greater or equal to %zu bytes", MINIMUM_ALIGNMENT_SIZE);
        return nullptr;
    }
    
    alloc_header* header = nullptr;
    int result = posix_memalign((void**)&header, alignment, sizeof(alloc_header) + size);

    if (result == EINVAL) {
        Logger::warning("The alignment argument was not a power of two, or was not a multiple of sizeof(void *).");
        goto cleanup;
    } else if (result == ENOMEM) {
        Logger::warning("There was insufficient memory to fulfill the allocation request.");
        goto cleanup;
    }

    memset(header, 0, sizeof(alloc_header) + size);
    header->allocation_size = size;
    header->alignment = alignment;

    if (!platform_ptr) {
        Logger::warning("Allocating %zu bytes before initializing platform", size);
    } else {
        Logger::warning("Allocating %zu bytes, total: %zu", size, platform_ptr->m_TotalAllocation);
        platform_ptr->m_TotalAllocation += size;
    }  

    return from_header_to_memory(header);    

cleanup:
    if (header) {
        AFree(header);
    }
    return nullptr;
}

void Platform::AFree(void* memory) {
    alloc_header* header = from_memory_to_header(memory);

    if (!platform_ptr) {
        Logger::warning("Freeing %zu bytes before initializing platform", header->allocation_size);
    } else {
        Logger::warning("Freeing %zu bytes, total: %zu", header->allocation_size, platform_ptr->m_TotalAllocation);
        platform_ptr->m_TotalAllocation -= header->allocation_size;
    }

    memset(header, 0, sizeof(alloc_header) + header->allocation_size);
    free(header);
}

void* Platform::ZeroMemory(void* memory, size_t size) {
    return memset(memory, 0, size);
}

void Platform::log(log_level level, const char *message) {
    static constexpr const char* color_string[] = { "0;41", "1;33", "1;32", "1;30" };
    printf("\033[%sm%s\033[0m\n", color_string[level], message);
}

void* Platform::LoadLibrary(const char* libraryPath) {
    char library_name[1024]{};

    char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) == nullptr) {
        Logger::warning("Platform::load_library: Failed to get current working directory");
        return nullptr;
    }

    snprintf(library_name, sizeof(library_name), "%s/lib%s.so", path, libraryPath);

    void* library = dlopen(library_name, RTLD_NOW);

    if (!library) {
        Logger::fatal("%s", dlerror());
    }

    return library;
}

void Platform::UnloadLibrary(void* library) {
    dlclose(library);
}

void* Platform::LoadLibraryFunction(void* library, const std::string& functionName) {
    void* function_ptr = dlsym(library, functionName.c_str());

    const char* error_message = dlerror();
    
    if (error_message) {
        Logger::warning("%s", error_message);
        return nullptr;
    }

    return function_ptr;
}

void* Platform::create_vulkan_surface(Window* window, void* instance) {
    VkSurfaceKHR surface = 0;
    
    if (SDL_Vulkan_CreateSurface((SDL_Window*)window->get_internal_handle(), (VkInstance)instance, &surface) != SDL_TRUE) {
        Logger::fatal("Failed to create vulkan surface");
        return nullptr;
    }

    return surface;
}

binary_info Platform::OpenBinary(const char* path) {
    FILE* file = fopen(path, "rb");

    if (!file) {
        Logger::warning("Failed to open file %s", path);
        return {};
    }

    binary_info info{};

    // go to the end of the file
    fseek(file, 0, SEEK_END);
    // get the size of the file
    info.size = ftell(file);
    // get back to the beginning
    fseek(file, 0, SEEK_SET);

    info.binary = (char*)Platform::UAlloc(info.size);

    fread(info.binary, info.size, 1, file);

    fclose(file);

    return info;
}

void Platform::CloseBinary(binary_info* binary_info) {
    Platform::UFree(binary_info->binary);
    binary_info->binary = 0;
    binary_info->size = 0;
}

int64_t Platform::GetTime() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return int64_t(now.tv_sec) * int64_t(1000000000) + int64_t(now.tv_nsec);
}

#endif