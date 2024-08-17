#include "game.h"

#include <core/application.h>
#include <core/logger.h>
#include <core/string.h>
#include <platform/platform.h>
#include <window/window.h>
#include <core/image_loader.h>
#include <core/image.h>

#include <cstdint>
#include <cstring>
#include <iterator>
#include <DirectXMath.h>

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 texCoord;
};

Game::Game(const Application* application)
	:
	m_Application(application) {}

Game::~Game() {
	Logger::Debug("Destroying game instance");
}

void Game::OnBegin() {
	Logger::Debug("OnBegin");

    CreateTestPlane();
}

void Game::OnUpdate(float deltaTime) {
	// const float move_factor = 50.0f * deltaTime;
    // if (m_Application->GetWindow()->IsMouseConfined()) {
    //     if (m_Application->GetWindow()->IsKeyPressed(Key::Key_W)) {
    //         m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, 0.0f, move_factor));
    //     }
    //     if (m_Application->GetWindow()->IsKeyPressed(Key::Key_S)) {
    //         m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, 0.0f, -move_factor));
    //     }
    //     if (m_Application->GetWindow()->IsKeyPressed(Key::Key_A)) {
    //         m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(-move_factor, 0.0f, 0.0f));
    //     }
    //     if (m_Application->GetWindow()->IsKeyPressed(Key::Key_D)) {
    //         m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(move_factor, 0.0f, 0.0f));
    //     }
    //     if (m_Application->GetWindow()->IsKeyPressed(Key::Key_SPACE)) {
    //         m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, move_factor, 0.0f));
    //     }
    //     if (m_Application->GetWindow()->IsKeyPressed(Key::Key_LCTRL)) {
    //         m_Application->GetRenderer()->OffsetCameraPosition(DirectX::XMFLOAT3(0.0f, -move_factor, 0.0f));
    //     }
    // }

    // GeometryRenderData render_data;

    // static float time = 0.0f;
    // time += deltaTime;

    // DirectX::XMStoreFloat4x4(&render_data.model, DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, time));

    // m_Application->GetRenderer()->UpdateRenderItem(m_RenderItem[0], &render_data);
}

void Game::OnShutdown() {
	Logger::Debug("OnShutdown");

	//RendererBackend* renderer = m_Application->GetRenderer();
}

void Game::CreateTestPlane() {
	
}
