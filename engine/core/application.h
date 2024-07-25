#pragma once

class IGame;
class Renderer;
class Window;
class Platform;

class RAPI Application {
public:
	Application();
	~Application();

	inline void SetGame(IGame* game) { m_Game = game; }
	inline Renderer* GetRenderer() const { return m_Renderer; }
	inline Window* GetWindow() const { return m_Window; }

	int Run();

private:
	IGame* m_Game = nullptr;
	Platform* m_Platform = nullptr;
	Renderer* m_Renderer = nullptr;
	Window* m_Window = nullptr;
};