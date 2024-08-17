#pragma once

#include <cstdint>

#include "defines.h"
#include "containers/list.h"
#include "window/key_defines.h"

struct SDL_Window;
union SDL_Event;

class RAPI Window {
public:
    Window(int32_t x, int32_t y, int32_t width, int32_t height, const char* name);
    ~Window();

    int ProcessMessages();
    
    static void MessageBox(const char* title, const char* message);

    void GetDimensions(uint32_t* width, uint32_t* height) const;

    bool ConfineCursorToWindow();
    bool FreeCursorFromWindow();
    bool IsMouseConfined() const;
    AINLINE bool IsKeyPressed(Key key) const { return m_KeyState[(int)key]; }
    AINLINE void* GetWindowInternalHandle() const { return m_Window; }

private:
    void process_window_messages(const void* pEvent);
    void process_key_event(const void* pKey, bool pressed);
    void process_mouse_motion(const void* pMotion);
    void process_mouse_confinment();

private:
    SDL_Window* m_Window;
    bool m_IsRunning = false;
    bool m_IsMouseHiddenByUser = false;
    bool m_IsMouseHidden = false;
    const uint8_t* m_KeyState = nullptr;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
};