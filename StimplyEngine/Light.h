#pragma once
#include "Drawable.h"
#include "Buffer.h"

class Light : public Drawable
{
public:
	Light();

	virtual void Update() override;
	virtual void Draw() override;
	DirectX::XMFLOAT4 GetLightPos() { return m_LightPos; }

private:
	DirectX::XMFLOAT4 m_LightPos = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4X4 m_Matrix;
	std::unique_ptr<Buffer<DirectX::XMFLOAT4X4>> m_CBuf;
};

