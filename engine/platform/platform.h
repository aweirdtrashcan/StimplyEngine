#pragma once

#include "core/logger.h"
#include "core/string.h"

#include <cstddef>
#include <cstdint>

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

    static void Log(log_level level, const char* message);

    static void* LoadLibrary(const char* libraryPath);
    static void UnloadLibrary(void* library);
    static void* LoadLibraryFunction(void* library, const char* functionName);

    static void* CreateVulkanSurface(const Window* window, void* instance);
    static list<const char*> GetRequiredExtensionNames(const Window& window);

    [[nodiscard("binary_info contains a dynamically allocated string")]] 
    static binary_info OpenBinary(const char* path);
    static void CloseBinary(binary_info* binary_info);

    /* Returns the current time in nanoseconds */
    static int64_t GetTime();

    static String GetCurrentWorkingDirectory();

    /* Construct an object with Args...*/
    template<typename Type, typename... Args>
    static AINLINE Type* Construct(Args&&... args) {
        void* memory = UAlloc(sizeof(Type));
        try {
            new(memory) Type(std::forward<Args>(args)...);
        } catch (...) {
            UFree(memory);
            memory = 0;
            throw;
        }
        return (Type*)memory;
    }

    /* Construct an object with Arg */
    template<typename Type, typename Arg>
    static AINLINE Type* Construct(Arg&& arg) {
        void* memory = UAlloc(sizeof(Type));
        try {
            new (memory) Type(std::forward<Arg>(arg));
        } catch (...) {
            UFree(memory);
            memory = 0;
            throw;
        }
        return (Type*)memory;
    }

    /* Construct an object without args */
    template<typename Type>
    static AINLINE Type* Construct() {
        void* memory = UAlloc(sizeof(Type));
        try {
            new (memory) Type();
        } catch (...) {
            UFree(memory);
            memory = 0;
            throw;
        }
        return (Type*)memory;
    }

    /* Destroy an object */
    template<typename Type>
    static AINLINE void Destroy(Type* object) {
        if (object) {
            object->~Type();
            UFree(object);
        }
    }

private:
    static inline Platform* platform_ptr = nullptr;
    /* HACK: Temporary */
    static inline uint8_t* m_BaseLinearMemory = nullptr;
    static inline uint8_t* m_CurrentLinearMemory = nullptr;
    size_t m_TotalAllocation = 0;
};