#include "vulkan_internals.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

bool create_surface(internal_vulkan_renderer_state* state) {
    SDL_Vulkan_CreateSurface((SDL_Window*)state->window, state->instance, &state->surface);

    if (state->surface == nullptr) {
        return false;
    }

    return true;
}