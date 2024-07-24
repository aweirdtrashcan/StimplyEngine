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

        //void* render_item = renderer.CreateRenderItem(&create_info);

        while (window.ProcessMessages()) {
            if (!renderer.Draw()) {
                Logger::fatal("Some error happened while Drawing, closing the engine...");
                break;
            }
        }

        //renderer.DestroyRenderItem(render_item);
        
    } catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
    }
}