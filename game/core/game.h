#pragma once

#include <core/game_interface.h>

class Game : public IGame {
public:
	virtual void OnBegin() override;
	virtual void OnUpdate() override;
	virtual void OnShutdown() override;
};