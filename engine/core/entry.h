#pragma once

#include "application.h"

class IGame;

extern void InitializeGame(IGame** out_game, const Application* application);

int main(void) {
	IGame* game = nullptr;

	Application app;

	InitializeGame(&game, &app);

	app.SetGame(game);

	return app.Run();
}