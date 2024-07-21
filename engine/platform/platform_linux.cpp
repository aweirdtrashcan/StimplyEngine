#include "SDL2/SDL_messagebox.h"
#include "platform.h"

#if defined (PLATFORM_LINUX)

#include "window/window.h"
#include "core/logger.h"

#include <SDL2/SDL_vulkan.h>

#include <dlfcn.h>
#include <unistd.h>
#include <linux/limits.h> // NOTE: I think you must have linux-headers installed, but i still need to look for that up.

void Window::MessageBox(const char* title, const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, nullptr);
}

struct alloc_header {
    size_t allocation_size;
};

static Platform* platform_ptr = nullptr;

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

void Platform::log(log_level level, const char *message) {
    static constexpr const char* color_string[] = { "0;41", "1;33", "1;32", "1;30" };
    printf("\033[%sm%s\033[0m\n", color_string[level], message);
}

void* Platform::load_library(const char* libraryPath) {
    char library_name[1024]{};

    char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) == nullptr) {
        Logger::debug("Platform::load_library: Failed to get current working directory");
        return nullptr;
    }

    snprintf(library_name, sizeof(library_name), "%s/lib%s.so", path, libraryPath);

    void* library = dlopen(library_name, RTLD_NOW);

    if (!library) {
        Logger::fatal("%s", dlerror());
    }

    return library;
}

void Platform::unload_library(void* library) {
    dlclose(library);
}

void* Platform::load_library_function(void* library, const std::string& functionName) {
    return dlsym(library, functionName.c_str());
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
    FILE* file = fopen(path, "rb");

    if (!file) return {0, nullptr};

    binary_info info{};

    // go to the end of the file
    fseek(file, 0, SEEK_END);
    // get the size of the file
    info.size = ftell(file);
    // get back to the beginning
    fseek(file, 0, SEEK_SET);

    info.binary = (char*)Platform::ualloc(info.size);

    fread(info.binary, info.size, 1, file);

    fclose(file);

    return info;
}

#endif