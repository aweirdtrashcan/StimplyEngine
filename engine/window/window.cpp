#include "window.h"

#include "core/event_types.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "core/event.h"

#include <SDL2/SDL.h>
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

    //SDL_ShowWindow(m_Window);
    m_KeyState = SDL_GetKeyboardState(nullptr);

    int32_t new_width;
    int32_t new_height;

    SDL_GetWindowSize(m_Window, &new_width, &new_height);

    m_Width = new_width;
    m_Height = new_height;

    m_IsRunning = true;
}

Window::~Window() {
    m_KeyState = nullptr;
    SDL_DestroyWindow(m_Window);
}

int Window::ProcessMessages() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if (event.type == SDL_EventType::SDL_QUIT) {
            m_IsRunning = false;
        }
        process_window_messages(&event);
    }

    process_mouse_confinment();

    return m_IsRunning;
}

void Window::GetDimensions(uint32_t* width, uint32_t* height) const {
    if (!width || !height) {
        Logger::Warning("Window::GetDimensions: width or height are nullptr");
        return;
    }

    *width = m_Width;
    *height = m_Height;
}

bool Window::ConfineCursorToWindow() {
    int result = SDL_SetRelativeMouseMode(SDL_TRUE);

    if (result) {
        Logger::Warning("Failed to confine cursor to Window: %s", SDL_GetError());
        return false;
    }

    m_IsMouseHiddenByUser = true;

    return true;
}

bool Window::FreeCursorFromWindow() {
    int result = SDL_SetRelativeMouseMode(SDL_FALSE);

    if (result) {
        Logger::Warning("Failed to free cursor from Window: %s", SDL_GetError());
        return false;
    }

    m_IsMouseHiddenByUser = false;

    return true;
}

bool Window::IsMouseConfined() const {
    return SDL_GetRelativeMouseMode() == SDL_TRUE;
}

void Window::process_window_messages(const void* pEvent) {
    const SDL_Event& event = *(SDL_Event*)pEvent;
    switch (event.type) {
        case SDL_KEYDOWN: {
            process_key_event(&event.key.keysym, true);
            break;
        }
        case SDL_KEYUP: {
            process_key_event(&event.key.keysym, false);
            break;
        }
        case SDL_MOUSEMOTION: {
            process_mouse_motion(&event.motion);
            break;
        }
        case SDL_WINDOWEVENT: {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                SDL_GetWindowSize(m_Window, (int*)&m_Width, (int*)&m_Height);

                WindowResizedEventData eventData;
                eventData.width = m_Width;
                eventData.height = m_Height;

                IEvent::FireEvent(EventType::WindowResized, &eventData);
            }
            break;
        }
        default: break;
    }
}

void Window::process_key_event(const void* pKey, bool pressed) {
    const SDL_Keysym& key = *(SDL_Keysym*)pKey;
    
    KeyboardEventData eventData;
    // TODO: Translate to engine-specific
    eventData.Key = (Key)key.scancode;
    eventData.Pressed = pressed;

    IEvent::FireEvent(EventType::KeyboardEvent, &eventData);
}

void Window::process_mouse_motion(const void* pMotion) {
    const SDL_MouseMotionEvent& event = *(SDL_MouseMotionEvent*)pMotion;

    // Logger::Warning("Mouse coordinates | Mouse Relative");
    // Logger::Warning("%i %i             | %i %i", event.x, event.y, event.xrel, event.yrel);

    MouseEventData eventData;
    eventData.MouseXMotion = event.xrel;
    eventData.MouseYMotion = event.yrel;
    eventData.MouseXScreen = event.x;
    eventData.MouseYScreen = event.y;

    IEvent::FireEvent(EventType::MouseMoved, &eventData);
}

void Window::process_mouse_confinment() {
    // check if window has input focus
    uint32_t flags = SDL_GetWindowFlags(m_Window);

    if (flags & SDL_WINDOW_INPUT_FOCUS) {
        if (m_IsMouseHiddenByUser && !m_IsMouseHidden) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_IsMouseHidden = true;
        }
    } else {
        if (m_IsMouseHiddenByUser && m_IsMouseHidden) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            m_IsMouseHidden = false;
        }
    }
}
