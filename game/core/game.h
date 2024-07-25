#pragma once


class Game {
public:
	Game() = default;
	virtual ~Game() = default;

	void OnBegin();
	void OnUpdate();
	void OnShutdown();
};