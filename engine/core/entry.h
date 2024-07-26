#pragma once

#include "application.h"

class IGame;

extern void InitializeGame(IGame** out_game, const Application* application);
extern void ShutdownGame(IGame* game);

int main(void) {
	IGame* game = nullptr;
	Application app;

	InitializeGame(&game, &app);

	app.SetGame(game);

	int ret_val = app.Run();

	ShutdownGame(game);

	return ret_val;
}