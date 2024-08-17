#pragma once

class IGame;
class RendererFrontend;
class RendererBackend;
class Window;
class Platform;

class RAPI Application {
public:
	Application();
	~Application();

	inline void SetGame(IGame* game) { m_Game = game; }
	inline RendererFrontend* GetRenderer() const { return m_Renderer; }
	inline const Window* GetWindow() const { return m_Window; }

	int Run();

private:
	IGame* m_Game = nullptr;
	Platform* m_Platform = nullptr;
	RendererFrontend* m_Renderer = nullptr;
	Window* m_Window = nullptr;
	float m_DeltaTime = 0.0f;
	RendererBackend* m_RendererBackend = nullptr;
};