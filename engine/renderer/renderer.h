#pragma once

#include "renderer/renderer_types.h"
#include "renderer_interface.h"
#include "core/event.h"

#include <DirectXMath.h>

class Window;
struct RenderItemCreateInfo;

enum class RendererType : char {
    D3D12,
    VULKAN
};

class RAPI Renderer : public IEvent {
public:
    Renderer(RendererType type, Window* window);
    ~Renderer();

    virtual void OnEvent(EventType type, const EventData* pEventData) override;

    bool Draw();
    inline HANDLE CreateRenderItem(const RenderItemCreateInfo* renderItem) { return m_Interface.renderer_create_render_item(renderItem); }
    inline void DestroyRenderItem(HANDLE renderItem) { m_Interface.renderer_destroy_render_item(renderItem); }
    inline void SetViewProjection(DirectX::XMMATRIX view, DirectX::CXMMATRIX projection) { m_Interface.renderer_set_view_projection(view, projection); }
    inline void SetRenderItemModel(HANDLE renderItem, const DirectX::XMFLOAT4X4* model) { m_Interface.renderer_set_render_item_model(renderItem, model); }
    void OffsetCameraPosition(DirectX::XMFLOAT3 offset);
    inline Texture CreateTexture(const char* name, bool auto_release, uint32_t width, uint32_t height, 
                                uint32_t channel_count, const uint8_t* pixels, bool has_transparency) {
        return m_Interface.renderer_create_texture(name, auto_release, width, height, channel_count, pixels, has_transparency);
    }
    inline void DestroyTexture(Texture texture) { m_Interface.renderer_destroy_texture(texture); }

private:
    void CalculateViewMatrix();
    [[nodiscard]] renderer_interface LoadRendererFunctions(RendererType type);

private:
    renderer_interface m_Interface;
    HANDLE m_Library = nullptr;
    static inline renderer_state m_Renderer_Memory = nullptr;
    Window* m_Window;

    DirectX::XMFLOAT4 m_EyePosition = DirectX::XMFLOAT4(0.0f, 0.0f, -30.0f, 0.0f);
    DirectX::XMFLOAT4 m_FocusPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT4 m_UpDirection = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
    
    float m_CameraPitch = 0.0f;
    float m_CameraYaw = 0.0f;

    float m_AspectRatio = 16.f / 9.f;
    float m_NearZ = 0.1f;
    float m_FarZ = 1000.f;
    float m_Fov = 45.f;
};
