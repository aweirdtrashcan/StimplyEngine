#pragma once
#include "includes.h"
#include <vector>
#include "DxMacros.inl"
#include "EngineTypes.inl"

class Bindable
{
public:
	virtual void Bind() = 0;
	virtual ~Bindable() = default;

protected:
	// local copy of device and context
	inline static const DeviceContext* m_DeviceCtx;
};

