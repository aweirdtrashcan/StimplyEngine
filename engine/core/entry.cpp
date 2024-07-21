#include "renderer/render_item_utils.h"
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

        struct Vertex {
            float x;
            float y;
            float z;
        };

        Vertex vertices[] = {
            { -0.5f, -0.5f, 0.0f },
            { -0.0f, 0.5f, 0.0f },
            { 0.5f, -0.5f, 0.0f },
        };

        uint32_t indices[] = { 0, 1, 2 };

        RenderItemCreateInfo create_info;
        create_info.meshSize = sizeof(vertices);
        create_info.pMeshes = &vertices;
        create_info.indicesSize = sizeof(indices);
        create_info.pIndices = &indices;

        void* render_item = renderer.CreateRenderItem(&create_info);

        while (window.ProcessMessages()) {
            renderer.Draw();
        }

        renderer.DestroyRenderItem(render_item);
        
    } catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
    }
}