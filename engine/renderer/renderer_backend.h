#pragma once

#include "defines.h"
#include "renderer_exception.h"

class Window;
struct RenderItemCreateInfo;

class RendererBackend {
public:
    RendererBackend(const char* applicationName, const Window& window) 
        :
        m_ApplicationName(applicationName),
        m_Window(window)
    {}
    virtual ~RendererBackend() = default;

    virtual void Resized(uint32_t width, uint32_t height) = 0;
    virtual bool BeginFrame(float deltaTime) = 0;
    virtual bool EndFrame(float deltaTime) = 0;
    virtual void WaitDeviceIdle() = 0;

protected:
    const char* m_ApplicationName;
    const Window& m_Window;
    uint64_t m_NumFramesRendered = 0;
};
