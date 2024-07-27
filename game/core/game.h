#pragma once

#include <defines.h>
#include <core/game_interface.h>

class Application;

class Game : public IGame {
public:
	Game(const Application* application);
	~Game();
	
	virtual void OnBegin() override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnShutdown() override;

private:
	const Application* m_Application;
	HANDLE m_RenderItem = nullptr;
	HANDLE m_Texture;
};