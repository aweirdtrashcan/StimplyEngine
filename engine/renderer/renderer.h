#pragma once

#include "renderer_interface.h"

class Window;
struct RenderItemCreateInfo;

enum class RendererType : char {
    D3D12,
    VULKAN
};

class RAPI Renderer {
public:
    Renderer(RendererType type, Window* window);
    ~Renderer();

    bool Draw();
    void* CreateRenderItem(const RenderItemCreateInfo* render_item);
    void DestroyRenderItem(void* render_item);

private:
    [[nodiscard]] renderer_interface LoadRendererFunctions(RendererType type);

private:
    renderer_interface m_Interface;
    void* m_Library = nullptr;
    static inline renderer_state m_Renderer_Memory = nullptr;
};
