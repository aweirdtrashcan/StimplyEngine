#include "game.h"

#include <core/application.h>
#include <core/logger.h>
#include <renderer/renderer.h>
#include <window/window.h>

Game::Game(const Application* application)
	:
	m_Application(application) {}

Game::~Game() {
	Logger::debug("Destroying game instance");
}

void Game::OnBegin() {
	Logger::debug("OnBegin");

	Renderer* renderer = m_Application->GetRenderer();
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

    m_RenderItem = renderer->CreateRenderItem(&create_info);

	DirectX::XMFLOAT4X4 model;
    DirectX::XMStoreFloat4x4(&model, DirectX::XMMatrixIdentity());
    renderer->SetRenderItemModel(m_RenderItem, &model);
}

void Game::OnUpdate() {
	const float move_factor = 1.0f;
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
    }
}

void Game::OnShutdown() {
	Logger::debug("OnShutdown");

	Renderer* renderer = m_Application->GetRenderer();

	renderer->DestroyRenderItem(m_RenderItem);

}
