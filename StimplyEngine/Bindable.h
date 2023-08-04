#pragma once
#include "includes.h"
#include <vector>
#include "DxMacros.inl"
#include "EngineTypes.inl"
#include "GlobalContext.h"

class Bindable
{
public:
	virtual void Bind() = 0;
	virtual ~Bindable() {};
};

