#pragma once

#include <cstdint>

#include "platform/platform.h"
#include "containers/list.h"

struct SDL_Window;

class RAPI Window {
public:
    Window(int32_t x, int32_t y, int32_t width, int32_t height, const char* name);
    ~Window();

    int ProcessMessages();
    void* get_internal_handle() const { return m_Window; }
    
    list<const char*> get_vulkan_required_instance_layers() const;
    void* create_vulkan_surface(void* instance) { return Platform::create_vulkan_surface(this, instance); }
    static void MessageBox(const char* title, const char* message);

    void GetDimensions(int32_t* width, int32_t* height) const;

    bool ConfineCursorToWindow();
    bool FreeCursorFromWindow();
    bool IsMouseConfined() const;

private:
    void process_window_messages(const void* pEvent);
    void process_key_event(const void* pKey, bool pressed);
    void process_mouse_motion(const void* pMotion);

private:
    SDL_Window* m_Window;
    Platform m_Platform;
    bool m_IsRunning = false;
    bool m_IsMouseHiddenByUser = false;
    bool m_IsMouseHidden = false;

};