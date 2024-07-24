#pragma once

#include "renderer/renderer_types.h"
#include "renderer_interface.h"

#include <DirectXMath.h>

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
    inline void SetViewProjection(DirectX::XMMATRIX view, DirectX::CXMMATRIX projection) { m_Interface.renderer_set_view_projection(view, projection); }
    inline void SetRenderItemModel(HANDLE renderItem, const DirectX::XMFLOAT4X4* model) { m_Interface.renderer_set_render_item_model(renderItem, model); }

private:
    [[nodiscard]] renderer_interface LoadRendererFunctions(RendererType type);

private:
    renderer_interface m_Interface;
    HANDLE m_Library = nullptr;
    static inline renderer_state m_Renderer_Memory = nullptr;
    Window* m_Window;
};
