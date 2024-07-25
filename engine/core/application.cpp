#include "application.h"

#include "game_interface.h"
#include "DirectXMath.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "window/window.h"

#include <exception>
#include <DirectXMath/Extensions/DirectXMathAVX2.h>

Application::Application(class IGame* game)
	:
	m_Game(game) {}

Application::~Application() {
    delete m_Game;
}

int Application::Run() {
    // Logger::fatal("This is a test message!");
    // Logger::warning("This is a test message!");
    // Logger::debug("This is a test message!");
    // Logger::info("This is a test message!");

    m_Game->OnBegin();

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

        uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };

        RenderItemCreateInfo create_info;
        create_info.vertexSize = sizeof(vertices);
        create_info.pVertices = &vertices;
        create_info.verticesCount = std::size(vertices);
        create_info.indexSize = sizeof(indices);
        create_info.indicesCount = std::size(indices);
        create_info.pIndices = &indices;
        create_info.shader = nullptr;

        HANDLE render_item = renderer.CreateRenderItem(&create_info);

        DirectX::XMFLOAT4X4 model;
        DirectX::XMStoreFloat4x4(&model, DirectX::XMMatrixIdentity());
        renderer.SetRenderItemModel(render_item, &model);

        while (window.ProcessMessages()) {
            const float move_factor = 1.0f;

            m_Game->OnUpdate();

            if (window.IsMouseConfined()) {
                if (window.IsKeyPressed(Key::Key_W)) {
                    renderer.OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, 0.0f, move_factor));
                }
                if (window.IsKeyPressed(Key::Key_S)) {
                    renderer.OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, 0.0f, -move_factor));
                }
                if (window.IsKeyPressed(Key::Key_A)) {
                    renderer.OffsetCameraPosition(DirectX::XMFLOAT3(-move_factor, 0.0f, 0.0f));
                }
                if (window.IsKeyPressed(Key::Key_D)) {
                    renderer.OffsetCameraPosition(DirectX::XMFLOAT3(move_factor, 0.0f, 0.0f));
                }
            }

            if (!renderer.Draw()) {
                Logger::fatal("Some error happened while Drawing, closing the engine...");
                break;
            }
        }

        renderer.DestroyRenderItem(render_item);

    }
    catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
    }

    m_Game->OnShutdown();
    Logger::info("Leaving engine...");

	return 0;
}
