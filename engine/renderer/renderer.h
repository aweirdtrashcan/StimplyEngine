#pragma once

#include "renderer/renderer_types.h"
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
    inline HANDLE CreateRenderItem(const RenderItemCreateInfo* renderItem) { return m_Interface.renderer_create_render_item(renderItem); }
    inline void DestroyRenderItem(HANDLE renderItem) { m_Interface.renderer_destroy_render_item(renderItem); }
    
private:
    [[nodiscard]] renderer_interface LoadRendererFunctions(RendererType type);

private:
    renderer_interface m_Interface;
    HANDLE m_Library = nullptr;
    static inline renderer_state m_Renderer_Memory = nullptr;
};
