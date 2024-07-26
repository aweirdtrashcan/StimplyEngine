#include <core/entry.h>

#include "game.h"

void InitializeGame(IGame** out_game, const class Application* application) {
	// Game is allowed to change this, as long as the class inherits from IGame.
	*out_game = new Game(application);
}

void ShutdownGame(IGame* game) {
	delete game;
}