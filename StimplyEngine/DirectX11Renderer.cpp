#define TINYOBJLOADER_IMPLEMENTATION
#include "DirectX11Renderer.h"
#include "Window.h"
#include <sstream>
#include <ostream>
#include <string>
#include "Surface.h"
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <cstdlib>
#include <fstream>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "HumanModel.h"
#include "GlobalContext.h"
#include "Light.h"

using namespace Microsoft::WRL;

DirectX11Renderer::DirectX11Renderer(Window* window)
	:
	m_Window(window)
{
	CreateDevice();
	GetBackBuffers();
	CreateDepthBuffer();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	if (!ImGui_ImplWin32_Init(m_Window->GetHandle()))
	{
		Window::Log(ConsoleColor::Red, "Failed to init Win32 Impl");
	}
	if (!ImGui_ImplDX11_Init(m_Device.Get(), m_Context.Get()))
	{
		Window::Log(ConsoleColor::Red, "Failed to init Dx11 Impl");
	}

	Vector2D winSize = m_Window->GetWindowSize();
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Height = winSize.y;
	viewport.Width = winSize.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	D3D11_RECT scissor{};
	scissor.left = 0;
	scissor.top = 0;
	scissor.bottom = static_cast<LONG>(winSize.y);
	scissor.right = static_cast<LONG>(winSize.x);

	m_Context->RSSetViewports(1u, &viewport);
	m_Context->RSSetScissorRects(1u, &scissor);

	DirectX::XMStoreFloat4x4(&s_Projection, DirectX::XMMatrixPerspectiveLH(1.0f, winSize.y / winSize.x, 1.0f, 1500.f));
	GlobalContext::Initialize();
	GlobalContext::context = m_Context.Get();
	GlobalContext::device = m_Device.Get();
	GlobalContext::infoQueue = m_InfoQueue.Get();
}

DirectX11Renderer::~DirectX11Renderer()
{
	m_Light = 0;
	GlobalContext::Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
	for (Drawable* d : m_Drawables)
	{
		delete d;
	}
}

const LightConstantBuffer& DirectX11Renderer::GetLightConstantBuffer()
{
	return m_Light->GetLightConstantBuffer();
}

void DirectX11Renderer::CreateDevice()
{
	ComPtr<IDXGIFactory1> dxgiFactory;

	DXERR(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)), "Failed to create DXGI Factory.");
	
	SIZE_T maxMemory = 0;

	UINT deviceFlags = 0;

#ifdef _DEBUG
	deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_ADAPTER_DESC1 adapterDesc{};

	Vector2D winSize = m_Window->GetWindowSize();
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = s_NumFrames - 1;
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = m_Window->GetHandle();
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	DXERR(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		nullptr,
		0u,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		nullptr,
		&m_Context
	), "Failed to create device and swapchain");

	Window::Log(ConsoleColor::Green, "Created DX11 Device and Swapchain.");
	
	DXERR(m_Device.As(&m_InfoQueue), "Failed to get InfoQueue.");
}

bool DirectX11Renderer::BeginFrame(float deltaTime)
{
	static bool bInitialized = false;

	m_Context->OMSetRenderTargets(1u, m_RTV.GetAddressOf(), m_DepthStencilView.Get());

	const FLOAT color[] = { 0.9882352941176471f, 0.0117647058823529f, 0.6313725490196078f, 1.0f };
	m_Context->ClearRenderTargetView(m_RTV.Get(), color);
	m_Context->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (!bInitialized)
	{
		m_Light = new Light();
		m_Drawables.push_back(m_Light);
		m_Drawables.push_back(new HumanModel());
		bInitialized = true;
	}

	ControlCamera();

	return true;
}

bool DirectX11Renderer::EndFrame(float deltaTime)
{
	if (m_IsResizing) return true;
	//m_Context->DrawIndexed(indicesCount, 0u, 0u);
	
	for (Drawable* d : m_Drawables)
	{
		d->Draw();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	UINT presentFlags = syncInterval != 0 ? 0u : DXGI_PRESENT_ALLOW_TEARING;
	DXERR(m_SwapChain->Present(syncInterval, presentFlags), "Failed to present frame.");
	return true;
}

void DirectX11Renderer::GetBackBuffers(void)
{
	DXERR(m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&m_RTVResource)), "Failed to get back buffer");
	DXERR(m_Device->CreateRenderTargetView(m_RTVResource.Get(), nullptr, &m_RTV), "Failed to create RenderTargetView");
}

void DirectX11Renderer::CreateDepthBuffer(void)
{
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.ArraySize = 1u;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0u;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.Height = static_cast<UINT>(m_Window->GetWindowSize().y);
	depthDesc.Width = static_cast<UINT>(m_Window->GetWindowSize().x);
	depthDesc.MipLevels = 1u;
	depthDesc.MiscFlags = 0u;
	depthDesc.SampleDesc.Count = 1u;
	depthDesc.SampleDesc.Quality = 0u;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	
	DXERR(m_Device->CreateTexture2D(&depthDesc, nullptr, &m_BackBuffer), "Failed to create backbuffer.");
	
	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = FALSE;

	DXERR(m_Device->CreateDepthStencilState(&dsDesc, &m_DepthState), "Failed to create depth state");
	
	D3D11_DEPTH_STENCIL_VIEW_DESC dsView{};
	dsView.Texture2D.MipSlice = 0u;
	dsView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsView.Format = depthDesc.Format;

	DXERR(m_Device->CreateDepthStencilView(m_BackBuffer.Get(), &dsView, &m_DepthStencilView), "Failed to create depth stencil view");
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> DirectX11Renderer::CreateSamplerState()
{
	ComPtr<ID3D11SamplerState> sS;
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = 5.0f;

	DXERR(m_Device->CreateSamplerState(&samplerDesc, &sS), "Failed to create sampler state");

	return sS;
}

void DirectX11Renderer::ControlCamera()
{
	static bool bShow = true;
	ImGui::Begin("Camera Control", &bShow);
	if (bShow)
	{
		renderData.elapsedTime += ImGui::GetIO().DeltaTime;
		if (renderData.elapsedTime > 0.5f)
		{
			renderData.fps = (int)ImGui::GetIO().Framerate;
			renderData.elapsedTime = 0.0f;
		}
		ImGui::Text("Frametime: %.3f, FPS: %d", 1000.f / ImGui::GetIO().Framerate, renderData.fps);
		ImGui::Checkbox("Pause Movement", &renderData.pauseSin);
		ImGui::Checkbox("vSync", (bool*)&syncInterval);
		ImGui::SliderAngle("Camera Phi X", &renderData.phi, -180.0f, 180.0f);
		ImGui::SliderAngle("Camera Theta Y", &renderData.theta, -180.0f, 180.0f);
		ImGui::SliderAngle("Camera Distance", &renderData.camDist, -180.0f, -6.f);

		ImGui::SliderAngle("Camera Roll", &renderData.camRoll, -180.0f, 180.0f);
		ImGui::SliderAngle("Camera Pitch", &renderData.camPitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Camera Yaw", &renderData.camYaw, -180.0f, 180.0f);

		if (ImGui::Button("Reset"))
		{
			renderData.phi = renderData.theta = renderData.camRoll
				= renderData.camPitch = renderData.camYaw = 0.0f;
			renderData.camDist = DirectX::XMConvertToRadians(-15.0f);
		}
		ImGui::End();
	}
}

void DirectX11Renderer::ControlLight()
{
	
}

void DirectX11Renderer::SetupLight(bool& isSetup, RenderData::PixelCBuf* pPcb)
{
	/*static ComPtr<ID3D11Buffer> lightCBuf;
	static UINT vertCount = 0;
	
	if (!isSetup)
	{
		DirectX::XMFLOAT3 lightColor = { 1.0f, 1.0f, 1.0f };
		const Vertex vertBuffer[] =
		{
			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },

			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

			{ { -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

			{ { -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },

			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

			{ { -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },

			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

			{ {  0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		};
		vertCount = _countof(vertBuffer);

		ComPtr<ID3DBlob> vertBlob;
		ComPtr<ID3DBlob> pixelBlob;

		DXERR(D3DReadFileToBlob(L"VSLight.cso", &vertBlob), "Failed to compile VSLight");
		DXERR(D3DReadFileToBlob(L"PSLight.cso", &pixelBlob), "Failed to compile PSLight");

		ComPtr<ID3D11PixelShader> pixelShader;
		ComPtr<ID3D11VertexShader> vertShader;

		DXERR(m_Device->CreatePixelShader(
			pixelBlob->GetBufferPointer(),
			pixelBlob->GetBufferSize(),
			nullptr,
			&pixelShader), "Failed to create Light PixelShader");

		DXERR(m_Device->CreateVertexShader(
			vertBlob->GetBufferPointer(),
			vertBlob->GetBufferSize(),
			nullptr,
			&vertShader), "Failed to create Light VertexShader");

		m_Context->PSSetShader(pixelShader.Get(), nullptr, 0u);
		m_Context->VSSetShader(vertShader.Get(), nullptr, 0u);

		{
			D3D11_BUFFER_DESC bd{};
			bd.ByteWidth = sizeof(RenderData::PixelCBuf);
			bd.StructureByteStride = 0u;
			bd.MiscFlags = 0u;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			D3D11_SUBRESOURCE_DATA srd{};
			srd.pSysMem = pPcb;
			
			DXERR(m_Device->CreateBuffer(&bd, &srd, &lightCBuf), "Failed to create Light CBuf");

			m_Context->VSSetConstantBuffers(0u, 1u, lightCBuf.GetAddressOf());
		}
		
		{
			ComPtr<ID3D11Buffer> buffer;

			D3D11_BUFFER_DESC bd{};
			bd.ByteWidth = sizeof(vertBuffer);
			bd.StructureByteStride = sizeof(Vertex);
			bd.MiscFlags = 0u;
			bd.CPUAccessFlags = 0u;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_IMMUTABLE;
			D3D11_SUBRESOURCE_DATA srd{};
			srd.pSysMem = &vertBuffer;

			DXERR(m_Device->CreateBuffer(&bd, &srd, &buffer), "Failed to create Light CBuf");

			m_Context->IASetVertexBuffers(0u, 1u, buffer.GetAddressOf(), &bd.StructureByteStride, &bd.MiscFlags);
		}
		
		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{ "Color", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
		};

		ComPtr<ID3D11InputLayout> inputLayout;
		DXERR(m_Device->CreateInputLayout(
			ied,
			_countof(ied),
			vertBlob->GetBufferPointer(),
			vertBlob->GetBufferSize(),
			&inputLayout), "Failed to create light Input Layout");
		
		m_Context->IASetInputLayout(inputLayout.Get());
		isSetup = true;
	}

	if (ImGui::Begin("Light Pos"))
	{
		ImGui::SliderAngle("Light X", &pPcb->pos.x, -180.f, 180.f);
		ImGui::SliderAngle("Light Y", &pPcb->pos.y, -180.f, 180.f * 4);
		ImGui::SliderAngle("Light Z", &pPcb->pos.z, -180.f, 180.f);
		ImGui::End();
	}

	D3D11_MAPPED_SUBRESOURCE srd{};
	DXERR(m_Context->Map(lightCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &srd), "Failed to map light CBuf");
	srd.pData = pPcb;
	m_Context->Unmap(lightCBuf.Get(), 0u);
	m_Context->VSSetConstantBuffers(0u, 1u, lightCBuf.GetAddressOf());

	m_Context->Draw(36 * 3, 0u);*/
}
