#pragma once

#include "includes.h"
#include <vector>

#ifdef _DEBUG
#define DXERR(x, msg) if (m_InfoQueue) { m_InfoQueue->ClearStoredMessages(); GetError(); } if (FAILED(x)) { __debugbreak(); assert(false && msg); }
#else
#define DXERR(x, msg) (x)
#endif

struct Transforms
{
	DirectX::XMMATRIX model;
	DirectX::XMMATRIX MVP;
};

struct RenderData
{
	struct PixelCBuf
	{
		DirectX::XMFLOAT3 pos;
		float padding;
	} lightPos;
	float yPos;
	float phi;
	float theta;
	float camPitch;
	float camRoll;
	float camYaw;
	float camDist;
	float elapsedTime;
	bool pauseSin = false;
	int fps;
	float scaleX;
	float scaleY;
	float scaleZ;
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR focusPoint;
	DirectX::XMVECTOR upVec;
	DirectX::XMFLOAT4X4 camMat;
	DirectX::XMFLOAT4X4 projMat;
	float objRoll;
	float objPitch;
	float objYaw;
	float objZ;
	Transforms transforms;
	D3D11_MAPPED_SUBRESOURCE msr{};

	RenderData()
	{
		ZeroMemory(this, sizeof(RenderData));
		objZ = DirectX::XMConvertToRadians(30.f);
		scaleX = scaleY = scaleZ = 1.0f;
		camDist = DirectX::XMConvertToRadians(-15.f);
		lightPos = 
		{
			{ 0.0f, DirectX::XMConvertToRadians(2.0f), DirectX::XMConvertToRadians(1.0f) },
			0.0f
		};
	}
};

enum Shaders
{
	PixelShaderLightPosition = 0,
	VertexShaderMVP = 1,
	VertexShaderModel = 2,
	Shaders_MAX
};

class DirectX11Renderer
{
public:
	DirectX11Renderer(class Window* window);
	~DirectX11Renderer();

	bool BeginFrame(float deltaTime);
	bool EndFrame(float deltaTime);
	void ResizeWindow(uint32_t width, uint32_t height) 
	{
		m_IsResizing = true;
		m_Context->Flush();
		m_RTVResource.Reset();
		m_RTV.Reset();
		HRESULT hres = m_SwapChain->ResizeBuffers(s_NumFrames, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		GetBackBuffers();
		m_IsResizing = false;
	};

private:
	void CreateDevice(void);
	void GetBackBuffers(void);
	const char* GetError(void);
	void CreateDepthBuffer(void);
	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState();

private:
	bool m_IsResizing = false;
	static inline uint8_t s_NumFrames = 3u;
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_RTVResource;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
	Microsoft::WRL::ComPtr<ID3D11InfoQueue> m_InfoQueue;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_PixelConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_BackBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_RTextureView;
	UINT indicesCount = 0u;
	UINT syncInterval = 1u;

	class Window* m_Window;
	RenderData renderData;
};