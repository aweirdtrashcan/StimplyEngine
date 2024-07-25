#pragma once

class RAPI IGame {
public:
	virtual ~IGame() = default;
	virtual void OnBegin() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnShutdown() = 0;
};