#pragma once

#include "includes.h"
#include "Surface.h"
#include "GDIPlusManager.h"

typedef void(*second)();


class Application
{
public:
    Application(const wchar_t* applicationName, HINSTANCE hInstance);
    Application(const Application&) = delete;
    Application(Application&&) = default;

    ~Application();

    void Run();

    inline static GDIPlusManager* gdipm;

    void RegisterFuncForSecond(second second)
    {
        funcPtrs.push_back(second);
    }

private:
    class Window* _window = 0;
    class Renderer* _renderer = 0;
    const wchar_t* _applicationName;
    bool paused = false;
    std::vector<second> funcPtrs;

    std::chrono::high_resolution_clock clock;
    std::chrono::steady_clock::time_point t0;
    std::chrono::steady_clock::time_point t1;
    float deltaTime = 0.0f;
};

