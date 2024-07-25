#include "game.h"

#include <core/logger.h>

void Game::OnBegin() {
	Logger::debug("OnBegin");
}

void Game::OnUpdate() {
	
}

void Game::OnShutdown() {
	Logger::debug("OnShutdown");
}
