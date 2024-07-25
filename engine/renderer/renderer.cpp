#include "renderer.h"

#include "DirectXMath.h"
#include "renderer/global_uniform_object.h"
#include "renderer/renderer_types.h"
#include "renderer_interface.h"
#include "window/window.h"
#include "platform/platform.h"
#include "renderer_exception.h"

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

    m_Library = Platform::load_library(library_name);

    m_Interface = LoadRendererFunctions(type);

    if (!m_Interface.initialize) {
        throw RendererException("Failed to load library: %s", library_name);
    }     
    
    if (!m_Interface.initialize(&allocation_size, nullptr, "Stimply Engine", window->get_internal_handle())) {
        throw RendererException("Failed to get renderer required size for internal state");
    }

    m_Renderer_Memory = Platform::ualloc(allocation_size);

    if (!m_Interface.initialize(&allocation_size, m_Renderer_Memory, "Stimply Engine", window->get_internal_handle())) {
        throw RendererException("Failed to initialize renderer");
    }
}

Renderer::~Renderer() {
    m_Interface.shutdown();
    Platform::ufree(m_Renderer_Memory);
    Platform::unload_library(m_Library);
}

bool Renderer::Draw() {
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;

    DirectX::XMVECTOR eye_position = DirectX::XMLoadFloat4(&m_EyePosition);
    DirectX::XMVECTOR focus_position = DirectX::XMLoadFloat4(&m_FocusPosition);
    DirectX::XMVECTOR up_direction = DirectX::XMLoadFloat4(&m_UpDirection);
    
    int32_t width, height;
    m_Window->GetDimensions(&width, &height);

    // TODO: Move this logic to somewhere else
    m_AspectRatio = (float)width / (float)height;

    if (m_Interface.begin_frame() == FRAME_STATUS_SUCCESS) {
    
        projection = DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearZ, m_FarZ);
        view = DirectX::XMMatrixLookAtLH(eye_position, focus_position, up_direction);

        SetViewProjection(view, projection);
        
        if (m_Interface.renderer_draw_items() == FRAME_STATUS_FAILED) {
            Logger::warning("Failed to render draw items");
            return false;
        }
        
        if (m_Interface.end_frame() == FRAME_STATUS_FAILED) {
            Logger::warning("Failed to end frame");
            return false;
        }

        return true;
    }
    return false;
}

renderer_interface Renderer::LoadRendererFunctions(RendererType type) {
    renderer_interface interface{};

    std::string renderer_placeholder;

    if (type == RendererType::VULKAN) {
        renderer_placeholder = "vulkan";
    } else {
        renderer_placeholder = "d3d12";
    }

    interface.initialize = (PFN_renderer_backend_initialize)Platform::load_library_function(m_Library, renderer_placeholder + "_backend_initialize");
    interface.shutdown = (PFN_renderer_backend_shutdown)Platform::load_library_function(m_Library, renderer_placeholder + "_backend_shutdown");
    interface.create_texture = (PFN_create_texture)Platform::load_library_function(m_Library, renderer_placeholder + "_create_texture");
    interface.begin_frame = (PFN_renderer_begin_frame)Platform::load_library_function(m_Library, renderer_placeholder + "_begin_frame");
    interface.end_frame = (PFN_renderer_end_frame)Platform::load_library_function(m_Library, renderer_placeholder + "_end_frame");
    interface.renderer_create_render_item = (PFN_renderer_create_render_item)Platform::load_library_function(m_Library, renderer_placeholder + "_create_render_item");
    interface.renderer_destroy_render_item = (PFN_renderer_destroy_render_item)Platform::load_library_function(m_Library, renderer_placeholder + "_destroy_render_item");
    interface.renderer_draw_items = (PFN_renderer_draw_items)Platform::load_library_function(m_Library, renderer_placeholder + "_draw_items");
    interface.renderer_set_view_projection = (PFN_renderer_set_view_projection)Platform::load_library_function(m_Library, renderer_placeholder + "_set_view_projection");
    interface.renderer_set_render_item_model = (PFN_renderer_set_render_item_model)Platform::load_library_function(m_Library, renderer_placeholder + "_set_render_item_model");

    return interface;
}
