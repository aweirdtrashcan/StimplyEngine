#pragma once
#include "includes.h"
#include "DxMacros.inl"
#include "GlobalContext.h"
#include <assimp/Importer.hpp>

class Drawable
{
public:
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual ~Drawable() {};

protected:
	std::vector<std::unique_ptr<class Bindable>> m_Bindables;
	UINT m_IndicesCount = 0;
};
