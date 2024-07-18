#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "platform/platform.h"
#include "containers/list.h"

struct SDL_Window;

class RAPI Window {
public:
    Window(int32_t x, int32_t y, int32_t width, int32_t height, const char* name);
    ~Window();

    int process_messages();
    void* get_internal_handle() const { return m_Window; }
    
    list<const char*> get_vulkan_required_instance_layers() const;
    void* create_vulkan_surface(void* instance) { return Platform::create_vulkan_surface(this, instance); }

private:
    void process_window_messages(const SDL_Event& event);
    void process_key_event(SDL_Keysym key);
private:
    SDL_Window* m_Window;
    Platform m_Platform;
    bool m_IsRunning = false;
};