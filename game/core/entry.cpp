#include <core/entry.h>

#include "game.h"

void InitializeGame(IGame** out_game) {
	*out_game = new Game();
}