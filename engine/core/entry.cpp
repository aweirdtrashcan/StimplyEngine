#include "renderer/renderer.h"
#include "window/window.h"
#include "test.h"
#include "core/logger.h"
#include "renderer/renderer.h"

#include <exception>

RAPI void initialize_engine(void) {
    try {
        Window window(100, 100, 800, 600, "Stimply Engine");
        Renderer renderer(RendererType::VULKAN, &window);

        while (window.process_messages()) {
            if (!renderer.draw()) {
                break;
            }
        }
    } catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
    }
}