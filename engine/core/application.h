#pragma once

class RAPI Application {
public:
	Application(class IGame* game);
	~Application();

	int Run();

private:
	IGame* m_Game;
};