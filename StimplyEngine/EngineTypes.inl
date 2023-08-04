#pragma once

struct Vector2D
{
    float x;
    float y;
};

#define FIN __forceinline
#define MAYBE_INLINE inline

enum class BufferType : char
{
	VertexBuffer,
	IndexBuffer,
	ConstantBuffer
};

enum class ShaderStage : char
{
	VertexStage,
	PixelStage,
	UnknownOrNotUsed
};

#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>

/* pos, normals */
struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normals;
};

struct Matrices
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 mvp;
};

struct Float3Padding
{
	DirectX::XMFLOAT3 xyz;
	float padding;
};

struct DeviceContext
{
	friend bool operator==(const DeviceContext& a, const DeviceContext& b)
	{
		if (a.device != b.device)
			return false;
		if (a.context != b.context)
			return false;
		if (a.infoQueue != b.infoQueue)
			return false;
		
		return true;
	}

	DeviceContext(Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> infoQueue)
		:
		device(device),
		context(context),
		infoQueue(infoQueue)
	{}

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<ID3D11InfoQueue> infoQueue;
};

enum class TextureType : uint8_t
{
	Texture2D,
	Texture3D
};

struct LightConstantBuffer
{
	alignas(16)DirectX::XMFLOAT3 lightPos;
	alignas(16)DirectX::XMFLOAT3 materialColor;
	alignas(16)DirectX::XMFLOAT3 ambient;
	alignas(16)DirectX::XMFLOAT3 diffuseColor;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};