#include "stdafx.h"

#include "Application.h"
#include "Window.h"
#include "Renderer.h"
#include "EngineTypes.inl"
#include "EngineGlobals.h"
#include <thread>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

Application::Application(const wchar_t* applicationName, HINSTANCE hInstance)
    :
    _applicationName(applicationName)
{
    Vector2D initialPos = { 100, 100 };
    Vector2D initialSize = { 1280, 720 };
    _window = new Window(hInstance, initialPos, initialSize, L"StimplyClass");
    assert(_window != 0 && "Failed to create window");

    _renderer = new Renderer(_window, RendererBackendType::RENDERER_BACKEND_TYPE_D3D11);
    assert(_renderer && "Failed to create renderer.");

    if (!gdipm)
        gdipm = new GDIPlusManager();

    Global::gIsRunning = true;
}

Application::~Application()
{
    delete _window;
    delete _renderer;
    delete gdipm;
}

void Application::Run()
{
    while (Global::gIsRunning)
    {
        if (GetAsyncKeyState(VK_ESCAPE))
        {
            Global::gIsRunning = false;
        }
        static float elapsedTime = 0.0f;
        t0 = clock.now();
        _window->processMessages();        
        _renderer->BeginFrame(deltaTime);
        _renderer->EndFrame(deltaTime);
        t1 = clock.now();
        deltaTime = static_cast<float>((t1 - t0).count() * 1e-9);
    }
}