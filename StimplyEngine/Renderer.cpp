#include "stdafx.h"
#include "Renderer.h"
#include "Event.h"
#include "DirectX11Renderer.h"

Renderer::Renderer(Window* window, RendererBackendType type)
    :
    _window(window)
{
    _type = type;
    //if (type == RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN)
    //{
    //    _backend = reinterpret_cast<void*>(new VulkanRenderer(window));
    //}

    if (type == RendererBackendType::RENDERER_BACKEND_TYPE_D3D11)
    {
        _backend = reinterpret_cast<void*>(new DirectX11Renderer(window));
    }

    EventListener listener{};
    listener.type = EventType::WINDOW_RESIZE;
    listener.EventCallback = Renderer::NotifyWindowResize;

    Event::Subscribe(listener);
}

void Renderer::NotifyWindowResize(void* data)
{
    uint32_t width = ((uint32_t*)data)[0];
    uint32_t height = ((uint32_t*)data)[1];
    //if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN)
    //{
    //    ((VulkanRenderer*)_backend)->ResizeWindow(width, height);
    //}

    if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_D3D11)
    {
        ((DirectX11Renderer*)_backend)->ResizeWindow(width, height);
    }
}

Renderer::~Renderer()
{
    //if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN)
    //{
    //    VulkanRenderer* renderer = reinterpret_cast<VulkanRenderer*>(_backend);
    //    delete renderer;
    //}

    if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_D3D11)
    {
        DirectX11Renderer* renderer = (DirectX11Renderer*)_backend;
        delete renderer;
    }
}

void Renderer::BeginFrame(float deltaTime)
{
    //if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN)
    //{
    //    VulkanRenderer* renderer = (VulkanRenderer*)_backend;
    //    renderer->BeginFrame(deltaTime);
    //}
    if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_D3D11)
    {
        DirectX11Renderer* renderer = (DirectX11Renderer*)_backend;
        renderer->BeginFrame(deltaTime);
    }
}

void Renderer::EndFrame(float deltaTime)
{
    //if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN)
    //{
    //    VulkanRenderer* renderer = (VulkanRenderer*)_backend;
    //    renderer->EndFrame(0.0f);
    //}

    if (_type == RendererBackendType::RENDERER_BACKEND_TYPE_D3D11)
    {
        DirectX11Renderer* renderer = (DirectX11Renderer*)_backend;
        renderer->EndFrame(deltaTime);
    }
}
