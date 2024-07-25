#pragma once

#include "application.h"

class IGame;

extern void InitializeGame(IGame** out_game);

int main(void) {
	IGame* game = nullptr;

	InitializeGame(&game);

	Application app(game);

	return app.Run();
}