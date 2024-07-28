#include "game.h"
#include "DirectXMath.h"
#include "renderer/renderer_types.h"

#include <core/application.h>
#include <core/logger.h>
#include <cstdint>
#include <cstring>
#include <renderer/renderer.h>
#include <window/window.h>

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 texCoord;
};

Game::Game(const Application* application)
	:
	m_Application(application) {}

Game::~Game() {
	Logger::debug("Destroying game instance");
}

void Game::OnBegin() {
	Logger::debug("OnBegin");

    CreateTestPlane();
}

void Game::OnUpdate(float deltaTime) {
	const float move_factor = 50.0f * deltaTime;
    if (m_Application->GetWindow()->IsMouseConfined()) {
        if (m_Application->GetWindow()->IsKeyPressed(Key::Key_W)) {
            m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, 0.0f, move_factor));
        }
        if (m_Application->GetWindow()->IsKeyPressed(Key::Key_S)) {
            m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, 0.0f, -move_factor));
        }
        if (m_Application->GetWindow()->IsKeyPressed(Key::Key_A)) {
            m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(-move_factor, 0.0f, 0.0f));
        }
        if (m_Application->GetWindow()->IsKeyPressed(Key::Key_D)) {
            m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(move_factor, 0.0f, 0.0f));
        }
        if (m_Application->GetWindow()->IsKeyPressed(Key::Key_SPACE)) {
            m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, move_factor, 0.0f));
        }
        if (m_Application->GetWindow()->IsKeyPressed(Key::Key_LCTRL)) {
            m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, -move_factor, 0.0f));
        }
    }

    DirectX::XMFLOAT4X4 model;

    static float time = 0.0f;
    time += deltaTime;

    DirectX::XMStoreFloat4x4(&model, DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, time));

    m_Application->GetRenderer()->UpdateRenderItem(m_RenderItem[0], &model);
}

void Game::OnShutdown() {
	Logger::debug("OnShutdown");

	Renderer* renderer = m_Application->GetRenderer();

    renderer->DestroyTexture(m_Texture);
	renderer->DestroyRenderItem(m_RenderItem[0]);
	renderer->DestroyRenderItem(m_RenderItem[1]);

}

void Game::CreateTestPlane() {
	Renderer* renderer = m_Application->GetRenderer();

    Vertex vertices[] = {
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
        { { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
        { {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } }
    };

    uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };

    RenderItemCreateInfo create_info;
    create_info.vertexSize = sizeof(vertices);
    create_info.pVertices = &vertices;
    create_info.verticesCount = std::size(vertices);
    create_info.indexSize = sizeof(indices);
    create_info.indicesCount = std::size(indices);
    create_info.pIndices = &indices;

    constexpr uint64_t texSize = 800 * 600 * 4;
    uint8_t* pixels = new uint8_t[texSize];

    memset(pixels, 0, texSize);

    for (uint32_t i = 0; i < texSize; i += 4) {
        int& px = (int&)pixels[i];
        // setting all pixels to green.
        px = px | (0xff00ff00);
    }

    m_Texture = renderer->CreateTexture("Test texture", false, 800, 600, 4, pixels, false);

    create_info.texture = m_Texture;

    HANDLE render_item = renderer->CreateRenderItem(&create_info);
    m_RenderItem.push_back(render_item);
}
