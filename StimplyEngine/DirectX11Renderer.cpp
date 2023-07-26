#include "DirectX11Renderer.h"
#include "Window.h"
#include <sstream>
#include <ostream>
#include <string>
#include "Surface.h"
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

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
}

DirectX11Renderer::~DirectX11Renderer()
{
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
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
	
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 normal;
	};

	m_Context->OMSetRenderTargets(1u, m_RTV.GetAddressOf(), m_DepthStencilView.Get());

	const FLOAT color[] = { 0.9882352941176471f, 0.0117647058823529f, 0.6313725490196078f, 1.0f };
	m_Context->ClearRenderTargetView(m_RTV.Get(), color);
	m_Context->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (!bInitialized)
	{
		const float texScalar = 1.0f;
		const float coordScalar = 1.0f;
		Vertex vertexBuffer[] =
		{
			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },

			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },

			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },

			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

			{ { -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },

			{ { -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },

			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

			{ { -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },

			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

			{ {  0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		};

		unsigned short indices[36];
		for (unsigned short i = 0; i < 36; i++) { indices[i] = i; }
		for (Vertex& v : vertexBuffer)
		{
			v.color = { 1.0f, 0.f, 1.f };
		}
		indicesCount = _countof(indices);

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.CPUAccessFlags = 0u;
		vertexBufferDesc.ByteWidth = sizeof(vertexBuffer);
		vertexBufferDesc.StructureByteStride = sizeof(Vertex);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = vertexBuffer;

		DXERR(m_Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_VertexBuffer), "Failed to create vertex buffer");

		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.ByteWidth = sizeof(indices);
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.StructureByteStride = sizeof(unsigned short);
		
		D3D11_SUBRESOURCE_DATA srd{};
		srd.pSysMem = indices;
		
		DXERR(m_Device->CreateBuffer(&indexBufferDesc, &srd, &m_IndexBuffer), "Failed to create index buffer");

		{
			D3D11_BUFFER_DESC cBufferDesc{};
			cBufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR); // vector3 size.
			cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cBufferDesc.MiscFlags = 0;
			cBufferDesc.StructureByteStride = 0;

			DXERR(m_Device->CreateBuffer(&cBufferDesc, nullptr, &m_PixelConstantBuffer), "Failed to create constant buffer");
		}

		{
			D3D11_BUFFER_DESC cBufferDesc{};
			cBufferDesc.ByteWidth = sizeof(Transforms);
			cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cBufferDesc.MiscFlags = 0;
			cBufferDesc.StructureByteStride = 0;

			DXERR(m_Device->CreateBuffer(&cBufferDesc, nullptr, &m_ConstantBuffer), "Failed to create constant buffer");
		}

		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{ "Position", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "Color", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "Normal", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 24u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
		};

		ComPtr<ID3DBlob> m_PixelShaderBlob;
		ComPtr<ID3DBlob> m_VertexShaderBlob;

		DXERR(D3DReadFileToBlob(L"./PhongPS.cso", &m_PixelShaderBlob), "Failed to compile Pixel Shader.");
		DXERR(D3DReadFileToBlob(L"./PhongVS.cso", &m_VertexShaderBlob), "Failed to compile Vertex Shader.");

		DXERR(m_Device->CreatePixelShader(m_PixelShaderBlob->GetBufferPointer(), m_PixelShaderBlob->GetBufferSize(), nullptr, &m_PixelShader), "Failed to create shader");
		DXERR(m_Device->CreateVertexShader(m_VertexShaderBlob->GetBufferPointer(), m_VertexShaderBlob->GetBufferSize(), nullptr, &m_VertexShader), "Failed to create shader");

		DXERR(m_Device->CreateInputLayout(ied, _countof(ied), m_VertexShaderBlob->GetBufferPointer(), m_VertexShaderBlob->GetBufferSize(), &m_InputLayout), "Failed to create input layout");
		m_Context->IASetInputLayout(m_InputLayout.Get());

		m_Context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		m_Context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);

		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
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

		m_Context->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), &vertexBufferDesc.StructureByteStride, &vertexBufferDesc.CPUAccessFlags);
		m_Context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
		m_Context->VSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());

		// create texture

		const Surface s = Surface::FromFile("Images\\r.png");

		D3D11_TEXTURE2D_DESC rTextDesc{};
		rTextDesc.ArraySize = 1u;
		rTextDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		rTextDesc.CPUAccessFlags = 0u;
		rTextDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rTextDesc.Height = s.GetHeight();
		rTextDesc.Width = s.GetWidth();
		rTextDesc.MipLevels = 1u;
		rTextDesc.MiscFlags = 0u;
		rTextDesc.SampleDesc.Count = 1u;
		rTextDesc.SampleDesc.Quality = 0u;
		rTextDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA rTextSrd{};
		rTextSrd.pSysMem = s.GetBufferPtr();
		rTextSrd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);
		DXERR(m_Device->CreateTexture2D(&rTextDesc, &rTextSrd, &m_RTexture), "failed to create r-texture");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = rTextDesc.Format;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DXERR(m_Device->CreateShaderResourceView(m_RTexture.Get(), &srvDesc, &m_RTextureView), "Failed to create r-texture view");

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = 5.0f;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		DXERR(m_Device->CreateSamplerState(&samplerDesc, &m_SamplerState), "Failed to create sampler state");

		m_Context->PSSetSamplers(0u, 1u, m_SamplerState.GetAddressOf());
		m_Context->PSSetShaderResources(0u, 1u, m_RTextureView.GetAddressOf());

		bInitialized = true;
	}

	renderData.camDist = DirectX::XMConvertToRadians(-15.0f);

	if (ImGui::Begin("Camera Control"))
	{
		renderData.elapsedTime += ImGui::GetIO().DeltaTime;
		if (renderData.elapsedTime > 0.5f)
		{
			renderData.fps = ImGui::GetIO().Framerate;
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

	if (ImGui::Begin("Scaling"))
	{
		ImGui::SliderFloat("X", &renderData.scaleX, 0.f, 10.f);
		ImGui::SliderFloat("Y", &renderData.scaleY, 0.f, 10.f);
		ImGui::SliderFloat("Z", &renderData.scaleZ, 0.f, 10.f);
		ImGui::End();
	}

	if (!renderData.pauseSin)
	{
		renderData.yPos += 1.0f * deltaTime;
	}

	if (ImGui::Begin("Object Position"))
	{
		ImGui::SliderAngle("Object Roll", &renderData.objRoll, -180.f, 180.f);
		ImGui::SliderAngle("Object Pitch", &renderData.objPitch, -180.f, 180.f);
		ImGui::SliderAngle("Object Yaw", &renderData.objYaw, -180.f, 180.f);
		ImGui::SliderAngle("Object Z", &renderData.objZ, 0.0f, 360.f);
		ImGui::End();
	}
	
	renderData.transforms.model = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixRotationRollPitchYaw(renderData.objPitch, renderData.objYaw, renderData.objRoll) *
		DirectX::XMMatrixTranslation(0.0f, sin(renderData.yPos) * 0.5f, renderData.objZ) *
		DirectX::XMMatrixScaling(renderData.scaleX, renderData.scaleY, renderData.scaleZ) *
		DirectX::XMLoadFloat4x4(&renderData.projMat)
	);
	renderData.transforms.MVP = DirectX::XMMatrixTranspose(
		renderData.transforms.model *
		DirectX::XMLoadFloat4x4(&renderData.camMat) * 
		DirectX::XMLoadFloat4x4(&renderData.projMat)
	);

	if (ImGui::Begin("Light Pos"))
	{
		ImGui::SliderAngle("Light X", &renderData.lightPos.pos.x, -180.f, 180.f);
		ImGui::SliderAngle("Light Y", &renderData.lightPos.pos.y, -180.f, 180.f);
		ImGui::SliderAngle("Light Z", &renderData.lightPos.pos.z, -180.f, 180.f);
		ImGui::End();
	}

	DXERR(m_Context->Map(m_PixelConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &renderData.msr), "Failed to map constant buffer");
	memcpy(renderData.msr.pData, &renderData.lightPos, sizeof(renderData.lightPos));
	m_Context->Unmap(m_PixelConstantBuffer.Get(), 0u);

	DXERR(m_Context->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &renderData.msr), "Failed to map constant buffer");
	memcpy(renderData.msr.pData, &renderData.transforms, sizeof(renderData.transforms));
	m_Context->Unmap(m_ConstantBuffer.Get(), 0u);

	m_Context->VSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());
	m_Context->PSSetConstantBuffers(0u, 1u, m_PixelConstantBuffer.GetAddressOf());

	return true;
}

bool DirectX11Renderer::EndFrame(float deltaTime)
{
	if (m_IsResizing) return true;
	m_Context->DrawIndexed(indicesCount, 0u, 0u);
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

const char* DirectX11Renderer::GetError(void)
{
	UINT64 numStoredMsg = m_InfoQueue->GetNumStoredMessages();

	std::stringstream ss;

	std::string str;
	const char* strChar = 0;

	if (numStoredMsg)
	{
		for (UINT64 i = 0; i < numStoredMsg; ++i)
		{
			D3D11_MESSAGE message{};
			SIZE_T msgLength = 0;
			m_InfoQueue->GetMessageW(i, nullptr, &msgLength);
			m_InfoQueue->GetMessageW(i, &message, &msgLength);

			ss << message.pDescription << std::endl;
		}

		str = ss.str();
		strChar = str.c_str();

		MessageBoxA(nullptr, strChar, "Fatal error", MB_OK | MB_ICONEXCLAMATION);
	}

	if (strChar)
	{
		return strChar;
	}

	return "Unknown Error";
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
