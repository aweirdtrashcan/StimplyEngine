#include <core/entry.h>

#include "game.h"

void InitializeGame(IGame** out_game) {
	// Game is allowed to change this, as long as the class inherits from IGame.
	*out_game = new Game();
}