#include "renderer.h"

#include "core/event.h"
#include "core/event_types.h"
#include "core/logger.h"
#include "renderer_interface.h"
#include "window/key_defines.h"
#include "window/window.h"
#include "platform/platform.h"
#include "renderer_exception.h"

#include <DirectXMath.h>
#include <DirectXMath/Extensions/DirectXMathAVX2.h>

Renderer::Renderer(RendererType type, Window* window) 
    :
    m_Window(window) {
    uint64_t allocation_size = 0;

    // load the .dll/.so
    const char* library_name = nullptr;

    if (type == RendererType::VULKAN) {
        library_name = "Stimply-Renderer-Backend-Vulkan";
    } else {
        library_name = "Stimply-Renderer-Backend-DX12";
    }

    m_Library = Platform::LoadLibrary(library_name);

    m_Interface = LoadRendererFunctions(type);

    if (!m_Interface.renderer_initialize) {
        throw RendererException("Failed to load library: %s", library_name);
    }     
    
    if (!m_Interface.renderer_initialize(&allocation_size, nullptr, "Stimply Engine", window->get_internal_handle())) {
        throw RendererException("Failed to get renderer required size for internal state");
    }

    m_Renderer_Memory = Platform::UAlloc(allocation_size);

    if (!m_Interface.renderer_initialize(&allocation_size, m_Renderer_Memory, "Stimply Engine", window->get_internal_handle())) {
        throw RendererException("Failed to initialize renderer");
    }

    window->ConfineCursorToWindow();
    IEvent::RegisterListener(this, EventType::MouseMoved);
    IEvent::RegisterListener(this, EventType::KeyboardEvent);
}

Renderer::~Renderer() {
    m_Interface.renderer_shutdown();
    m_Window->FreeCursorFromWindow();
    IEvent::UnregisterListener(this, EventType::MouseMoved);
    Platform::UFree(m_Renderer_Memory);
    Platform::UnloadLibrary(m_Library);
}

void Renderer::OnEvent(EventType type, const EventData* pEventData) {
    switch (type) {
        case EventType::MouseMoved: {
            if (m_Window->IsMouseConfined()) {
                MouseEventData* eventData = (MouseEventData*)pEventData;
                m_CameraYaw += eventData->MouseXMotion / 0.25f * 0.0010f;
                m_CameraPitch += eventData->MouseYMotion / 0.25f * 0.0010f;
            }
            break;
        }
        case EventType::KeyboardEvent: {
            KeyboardEventData* eventData = (KeyboardEventData*)pEventData;

            if (eventData->Key == Key::Key_ESCAPE && eventData->Pressed) {
                if (m_Window->IsMouseConfined()) {
                    m_Window->FreeCursorFromWindow();
                } else {
                    m_Window->ConfineCursorToWindow();
                }
            }

            break;
        }
        default: break;
    }
}

bool Renderer::Draw() {
    CalculateViewMatrix();

    if (m_Interface.renderer_begin_frame() == FRAME_STATUS_SUCCESS) {       
        if (m_Interface.renderer_draw_items() == FRAME_STATUS_FAILED) {
            Logger::warning("Failed to render draw items");
            return false;
        }
        
        if (m_Interface.renderer_end_frame() == FRAME_STATUS_FAILED) {
            Logger::warning("Failed to end frame");
            return false;
        }

        return true;
    }
    return false;
}

void Renderer::CalculateViewMatrix() {
    int32_t width, height;
    m_Window->GetDimensions(&width, &height);

    // TODO: Move this logic to somewhere else
    m_AspectRatio = (float)width / (float)height;

    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;

    DirectX::XMVECTOR forward_vector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR camera_direction = DirectX::AVX2::XMVector3Transform(
        forward_vector, 
        DirectX::XMMatrixRotationRollPitchYaw(m_CameraPitch, m_CameraYaw, 0.0f));

    DirectX::XMVECTOR camera_position = DirectX::XMLoadFloat4(&m_EyePosition);
    DirectX::XMVECTOR focus_position = DirectX::XMVectorAdd(camera_position, camera_direction);
    DirectX::XMVECTOR up_direction = DirectX::XMLoadFloat4(&m_UpDirection);

    projection = DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearZ, m_FarZ);
    view = DirectX::XMMatrixLookAtLH(camera_position, focus_position, up_direction);

    SetViewProjection(view, projection);
}

void Renderer::OffsetCameraPosition(DirectX::XMFLOAT3 offset) {
    DirectX::XMMATRIX direction = DirectX::XMMatrixRotationRollPitchYaw(m_CameraPitch, m_CameraYaw, 0.0f);

    DirectX::XMFLOAT3 move_direction;
    DirectX::XMStoreFloat3(&move_direction, DirectX::AVX2::XMVector3Transform(DirectX::XMLoadFloat3(&offset), direction));

    m_EyePosition.x += move_direction.x;
    m_EyePosition.y += move_direction.y;
    m_EyePosition.z += move_direction.z;
}

inline void* format_symbol_name(char* buffer, uint64_t buffer_size, void* library, const char* renderer_placeholder, const char* symbol_name) {
    snprintf(buffer, buffer_size, "%s%s", renderer_placeholder, symbol_name);
    return Platform::LoadLibraryFunction(library, buffer);
}

#define load_library(symbol_name) format_symbol_name(buffer, sizeof(buffer), m_Library, renderer_placeholder, symbol_name)

renderer_interface Renderer::LoadRendererFunctions(RendererType type) {
    renderer_interface interface{};

    const char* renderer_placeholder = nullptr;
    char buffer[2048];
    
    if (type == RendererType::VULKAN) {
        renderer_placeholder = "vulkan";
    } else {
        renderer_placeholder = "d3d12";
    }

    interface.renderer_initialize = (PFN_renderer_backend_initialize)load_library("_backend_initialize");
    interface.renderer_shutdown = (PFN_renderer_backend_shutdown)load_library("_backend_shutdown");
    interface.renderer_begin_frame = (PFN_renderer_begin_frame)load_library("_begin_frame");
    interface.renderer_end_frame = (PFN_renderer_end_frame)load_library("_end_frame");
    interface.renderer_create_texture = (PFN_renderer_create_texture)load_library("_create_texture");
    interface.renderer_destroy_texture = (PFN_renderer_destroy_texture)load_library("_destroy_texture");
    interface.renderer_create_render_item = (PFN_renderer_create_render_item)load_library("_create_render_item");
    interface.renderer_destroy_render_item = (PFN_renderer_destroy_render_item)load_library("_destroy_render_item");
    interface.renderer_draw_items = (PFN_renderer_draw_items)load_library("_draw_items");
    interface.renderer_set_view_projection = (PFN_renderer_set_view_projection)load_library("_set_view_projection");
    interface.renderer_set_render_item_model = (PFN_renderer_set_render_item_model)load_library("_set_render_item_model");

    return interface;
}
