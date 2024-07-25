#include "DirectXMath.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "test.h"
#include "window/window.h"

#include <exception>
#include <DirectXMath/Extensions/DirectXMathAVX2.h>

RAPI void initialize_engine(void) {
    try {
        Window window(100, 100, 800, 600, "Stimply Engine");
        Renderer renderer(RendererType::VULKAN, &window);

        // TODO: Test
        DirectX::XMFLOAT3 vertices[] = {
            { -0.5f, -0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
        };

        uint32_t indices[] = {0, 1, 2, 2, 1, 3};

        RenderItemCreateInfo create_info;
        create_info.vertexSize = sizeof(vertices);
        create_info.pVertices = &vertices;
        create_info.verticesCount = std::size(vertices);
        create_info.indexSize = sizeof(indices);
        create_info.indicesCount = std::size(indices);
        create_info.pIndices = &indices;
        create_info.shader = nullptr;

        HANDLE render_item = renderer.CreateRenderItem(&create_info);

        float rotation_factor = 0.0f;

        while (window.ProcessMessages()) {
            rotation_factor += 0.0001f;
            DirectX::XMMATRIX model_mat = DirectX::XMMatrixIdentity();
            DirectX::XMMATRIX rotation_mat = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, rotation_factor);
            model_mat = DirectX::AVX2::XMMatrixMultiply(model_mat, rotation_mat);
                
            DirectX::XMFLOAT4X4 model;

            DirectX::XMStoreFloat4x4(&model, model_mat);

            renderer.SetRenderItemModel(render_item, &model);
            if (!renderer.Draw()) {
                Logger::fatal("Some error happened while Drawing, closing the engine...");
                break;
            }
        }

        renderer.DestroyRenderItem(render_item);

    } catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
    }

    Logger::info("Leaving engine...");
}