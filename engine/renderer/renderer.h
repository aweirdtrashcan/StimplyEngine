#pragma once

#include "renderer_interface.h"

enum class RendererType : char {
    D3D12,
    VULKAN
};

class Window;

class RAPI Renderer {
public:
    Renderer(RendererType type, class Window* window);
    ~Renderer();

    bool draw();

private:
    [[nodiscard]] renderer_interface load_renderer_functions(RendererType type);

private:
    renderer_interface m_Interface;
    void* m_Library = nullptr;
    static inline renderer_state m_Renderer_Memory = nullptr;
};
