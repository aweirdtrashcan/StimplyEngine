#include "renderer.h"

#include "renderer_interface.h"
#include "window/window.h"
#include "platform/platform.h"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>

class RendererException : public std::exception {
public:
    RendererException(const char* error) 
    :
    m_What(error) {}

    virtual const char* what() const noexcept { return m_What.c_str(); }

private:
    std::string m_What;
};

Renderer::Renderer(RendererType type, Window* window) {
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
        throw RendererException("Failed to load library");
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
    if (m_Interface.begin_frame()) {
        m_Interface.end_frame();
        return true;
    }
    return false;
}

void* Renderer::CreateRenderItem(const RenderItemCreateInfo* render_item) {
    return m_Interface.renderer_create_render_item(render_item);
}

void Renderer::DestroyRenderItem(void* render_item) {
    m_Interface.renderer_destroy_render_item(render_item);
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

    return interface;
}
