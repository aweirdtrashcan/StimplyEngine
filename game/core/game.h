#pragma once

#include <defines.h>
#include <core/game_interface.h>

#include <containers/list.h>

class Application;

class Game : public IGame {
public:
	Game(const Application* application);
	~Game();
	
	virtual void OnBegin() override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnShutdown() override;

	void CreateTestPlane();

private:
	const Application* m_Application;
	list<HANDLE> m_RenderItem;
	HANDLE m_Texture;
};