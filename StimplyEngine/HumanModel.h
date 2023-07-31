#pragma once
#include "Drawable.h"
#include "EngineTypes.inl"
#include <memory>
#include "Buffer.h"
#include <assimp/Importer.hpp>

class HumanModel : public Drawable
{
public:
	HumanModel(DeviceContext* drawableInfo);
	
	virtual void Update() override;
	virtual void Draw() override;
private:
	std::unique_ptr<class Buffer<Matrices>> m_MatrixBuffer;
	DirectX::XMFLOAT4X4 m_ModelMatrix;
	float scaleXYZ[3] = { 1.0f, 1.0f, 1.0f };
	float roll, pitch, yaw;
	float x, y, z;
	bool m_IsBound = false;
	UINT m_IndicesCount = 0;
	Matrices matrix;
	Assimp::Importer imp;
};

