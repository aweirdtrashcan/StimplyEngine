#pragma once

#include "includes.h"
#include "EngineTypes.inl"

enum class ConsoleColor
{
    Red,
    Green,
    Blue,

    Standard
};

class Window
{
public:
    Window(HINSTANCE hInstance, Vector2D initialPos, Vector2D initialSize, const wchar_t* className);
    ~Window();

    int processMessages()
    {
        MSG msg;
        int ret = TRUE;

        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return ret;
    }

    HWND GetHandle() const { return _hWnd; }
    Keyboard GetKeyboard() const { return _keyboard; }
    HINSTANCE GetInstance() const { return _hInstance; }
    Vector2D GetWindowSize() const { return _initialSize; }
    RECT GetWindowRect() const { return _windowRect; }
    void Throw(HRESULT reason);

    static void Log(ConsoleColor color, const char* message, ...);

private:
    static LRESULT CALLBACK WindowHandleSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND _hWnd = 0;
    HINSTANCE _hInstance = 0;
    const wchar_t* _className = 0;
    Keyboard _keyboard;
    static inline HANDLE s_Console_handle;
    Vector2D _initialSize;
    RECT _windowRect;
};

