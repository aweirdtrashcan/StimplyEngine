#pragma once
#include "Drawable.h"
#include "EngineTypes.inl"
#include <memory>
#include "Buffer.h"
#include <assimp/Importer.hpp>

class HumanModel : public Drawable
{
public:
	HumanModel();
	
	virtual void Update() override;
	virtual void Draw() override;
private:
	std::unique_ptr<class Buffer<Matrices>> m_MatrixBuffer;
	std::unique_ptr<class Buffer<DirectX::XMFLOAT4>> m_PixelShaderCBuf;
	DirectX::XMFLOAT4X4 m_ModelMatrix;
	float scaleXYZ[3] = { 1.0f, 1.0f, 1.0f };
	float roll, pitch, yaw;
	float x, y, z;
	Matrices matrix;
};

