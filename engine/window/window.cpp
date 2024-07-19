#include "window.h"

#include "core/logger.h"
#include "platform/platform.h"

#include <SDL2/SDL_vulkan.h>

Window::Window(int32_t x, int32_t y, int32_t width, int32_t height, const char* name) {
    m_Window = SDL_CreateWindow(
        name,
        x,
        y,
        width,
        height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    SDL_assert(m_Window != nullptr);

    SDL_ShowWindow(m_Window);

    m_IsRunning = true;
}

Window::~Window() {
    SDL_DestroyWindow(m_Window);
}

int Window::ProcessMessages() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if (event.type == SDL_EventType::SDL_QUIT) {
            m_IsRunning = false;
        }
        process_window_messages(event);
    }

    return m_IsRunning;
}

list<const char*> Window::get_vulkan_required_instance_layers() const {
    list<const char*> extensions;
    uint32_t extension_count = 0;

    SDL_Vulkan_GetInstanceExtensions(m_Window, &extension_count, nullptr);
    extensions.resize(extension_count);
    SDL_Vulkan_GetInstanceExtensions(m_Window, &extension_count, extensions.data());

    return extensions;
}

void Window::process_window_messages(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN: {
            process_key_event(event.key.keysym);
        }
        default: break;
    }
}

void Window::process_key_event(SDL_Keysym key) {
    switch (key.sym)
    {
    case SDLK_a:
        Logger::debug("Key A was pressed!");
        break;
    case SDLK_ESCAPE:
        m_IsRunning = false;
        break;
    default:
        break;
    }
}