#pragma once
#include "Drawable.h"

class Light : public Drawable
{
public:
	Light(DeviceContext* deviceCtx);

	virtual void Update() override;
	virtual void Draw() override;

private:

};

