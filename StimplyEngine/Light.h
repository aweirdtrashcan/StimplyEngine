#pragma once
#include "Drawable.h"
#include "Buffer.h"

class Light : public Drawable
{
public:
	Light();

	virtual void Update() override;
	virtual void Draw() override;
	const LightConstantBuffer& GetLightConstantBuffer() const { return m_LightCBuf; }

private:
	LightConstantBuffer m_LightCBuf;
	DirectX::XMFLOAT4X4 m_Matrix;
	std::unique_ptr<Buffer<DirectX::XMFLOAT4X4>> m_CBuf;
	std::unique_ptr<Buffer<DirectX::XMFLOAT4>> m_PSCbufLightColor;
};

