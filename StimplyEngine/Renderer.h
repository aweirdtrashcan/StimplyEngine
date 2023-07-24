#pragma once

#include "includes.h"

enum class RendererBackendType
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_D3D11
};

class Renderer
{
public:
    Renderer() = delete;
    Renderer(const Renderer&) = delete;
    Renderer(class Window* window, RendererBackendType type);

    static void NotifyWindowResize(void* EventData);

    ~Renderer();

    void BeginFrame(float);
    void EndFrame(float);

    bool IsIdle() { return true; }

private:
    Window* _window;

    // void* needs to be casted to whatever backend type is being used
    static inline void* _backend = 0;
    static inline RendererBackendType _type;
};

