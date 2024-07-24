#include "renderer/renderer.h"
#include "window/window.h"
#include "test.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "renderer/global_uniform_object.h"

#include <cstdint>
#include <exception>

RAPI void initialize_engine(void) {
    try {
        Window window(100, 100, 800, 600, "Stimply Engine");
        Renderer renderer(RendererType::VULKAN, &window);

        //void* render_item = renderer.CreateRenderItem(&create_info);

        while (window.ProcessMessages()) {
            int32_t width, height;
            window.GetDimensions(&width, &height);
            float aspect_ratio = (float)width / (float)height;
            GlobalUniformObject ubo;
    
            ubo.projection = DirectX::XMMatrixPerspectiveFovLH(45.f, aspect_ratio, 0.1f, 1000.f);

            DirectX::XMVECTOR eye_position = DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
            DirectX::XMVECTOR focus_position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR up_direction = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            ubo.view = DirectX::XMMatrixLookAtLH(eye_position, focus_position, up_direction);

            

            renderer.Draw();
        }

        //renderer.DestroyRenderItem(render_item);
        
    } catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
    }
}