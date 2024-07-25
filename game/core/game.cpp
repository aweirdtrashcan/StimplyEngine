#include "game.h"

#include <core/logger.h>

Game::Game() {
}

Game::~Game() {
	Logger::debug("Destroying game instance");
}

void Game::OnBegin() {
	Logger::debug("OnBegin");
}

void Game::OnUpdate() {
	
}

void Game::OnShutdown() {
	Logger::debug("OnShutdown");
}
