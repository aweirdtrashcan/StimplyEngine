#pragma once
#include "includes.h"
#include "DxMacros.inl"

struct DrawableInfo
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};

class Drawable
{
public:
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	static inline DeviceContext* m_DeviceCtx;
	std::vector<std::unique_ptr<class Bindable>> m_Bindables;
};
