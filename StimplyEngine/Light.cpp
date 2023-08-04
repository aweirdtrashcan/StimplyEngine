#include "Light.h"
#include "BindableIncludes.h"
#include <imgui.h>
#include "DirectX11Renderer.h"

Light::Light()
{
	std::vector<DirectX::XMFLOAT3> vertexBuffer =
	{
		{ -0.5f,  0.5f, 0.0f }, // top left - front
		{  0.5f, -0.5f, 0.0f }, // bottom right - front
		{ -0.5f, -0.5f, 0.0f }, // bottom left - front
		{  0.5f,  0.5f, 0.0f }, // top right - front
		{  0.5f, -0.5f, 1.0f }, // bottom right - left side
		{  0.5f,  0.5f, 1.0f }, // top right - left side
		{ -0.5f,  0.5f, 1.0f }, // top right - back side
		{ -0.5f, -0.5f, 1.0f }, // top bottom - back side
	};


	std::vector<unsigned short> indexBuffer =
	{
		0, 1, 2,
		0, 3, 1,
		1, 3, 4,
		3, 5, 4,
		4, 5, 7,
		5, 6, 7,
		7, 6, 2,
		6, 0, 2,
		0, 6, 3,
		6, 5, 3,
		2, 7, 1,
		7, 4, 1
	};

	m_IndicesCount = (UINT)indexBuffer.size();

	m_LightCBuf =
	{
		{ -5.054f, 2.88f, 56.101f },
		{ 0.7f, 0.7f, 0.7f },
		{ 0.05f, 0.05f, 0.05f },
		{ 1.0f, 1.0f, 1.0f },
		10.0f,
		1.0f,
		0.045f,
		0.0075f
	};

	m_Bindables.push_back(std::make_unique<Buffer<DirectX::XMFLOAT3>>(
		BufferType::VertexBuffer,
		ShaderStage::UnknownOrNotUsed,
		vertexBuffer
	));

	m_Bindables.push_back(std::make_unique<Buffer<unsigned short>>(
		BufferType::IndexBuffer,
		ShaderStage::UnknownOrNotUsed,
		indexBuffer
	));

	std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
	};

	m_Bindables.push_back(std::make_unique<PixelShader>(L"PSLight.cso"));

	std::unique_ptr<VertexShader> vShader = std::make_unique<VertexShader>(L"VSLight.cso");

	m_Bindables.push_back(std::make_unique<InputLayout>(ied, vShader->GetBlob().Get()));

	m_Bindables.push_back(std::move(vShader));

	m_Bindables.push_back(std::make_unique<PrimitiveTopology>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	std::vector<DirectX::XMFLOAT4X4> m = { DirectX::XMFLOAT4X4() };

	m_CBuf = std::make_unique<Buffer<DirectX::XMFLOAT4X4>>(
		BufferType::ConstantBuffer,
		ShaderStage::VertexStage,
		m
	);

	m_PSCbufLightColor = std::make_unique<Buffer<DirectX::XMFLOAT4>>(
		BufferType::ConstantBuffer,
		ShaderStage::PixelStage,
		std::vector<DirectX::XMFLOAT4> { DirectX::XMFLOAT4(m_LightCBuf.diffuseColor.x, m_LightCBuf.diffuseColor.y, m_LightCBuf.diffuseColor.z, 1.0f) }
	);
}

void Light::Update()
{
	static bool bShow = true;
	ImGui::Begin("Light Pos", &bShow);
	if (bShow)
	{
		ImGui::SliderFloat("Light X", &m_LightCBuf.lightPos.x, -10.f, 30.f);
		ImGui::SliderFloat("Light Y", &m_LightCBuf.lightPos.y, -10.f, 30.f);
		ImGui::SliderFloat("Light Z", &m_LightCBuf.lightPos.z, -10.f, 360.f);

		ImGui::Text("Intensity/Color");
		ImGui::SliderFloat("Diffuse Intensity", &m_LightCBuf.diffuseIntensity, 0.0f, 30.f);
		ImGui::ColorEdit3("Diffuse", (float*)&m_LightCBuf.diffuseColor);
		ImGui::ColorEdit3("Ambient", (float*)&m_LightCBuf.ambient);
		ImGui::ColorEdit3("Material", (float*)&m_LightCBuf.materialColor);

		ImGui::Text("Falloff");
		ImGui::SliderFloat("Constant", &m_LightCBuf.attConst, 0.05f, 2.0f);
		ImGui::SliderFloat("Linear", &m_LightCBuf.attLin, 0.0001f, 2.0f);
		ImGui::SliderFloat("Quadratic", &m_LightCBuf.attQuad, 0.00001f, 0.003f);

		if (ImGui::Button("Reset"))
		{
			m_LightCBuf =
			{
				{ 0.0f, 0.0f, 0.0f },
				{ 0.7f, 0.7f, 0.7f },
				{ 0.05f, 0.05f, 0.05f },
				{ 1.0f, 1.0f, 1.0f },
				1.0f,
				1.0f,
				0.045f,
				0.0075f
			};
		}
		ImGui::End();
	}

	const DirectX::XMFLOAT4X4& proj = DirectX11Renderer::GetProjection();

	DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixTranslation(m_LightCBuf.lightPos.x, m_LightCBuf.lightPos.y, m_LightCBuf.lightPos.z) *
		DirectX::XMLoadFloat4x4(&proj)
	);

	DirectX::XMStoreFloat4x4(&m_Matrix, m);

	m_CBuf->Update(&m_Matrix);
	DirectX::XMFLOAT4 f = { m_LightCBuf.diffuseColor.x, m_LightCBuf.diffuseColor.y, m_LightCBuf.diffuseColor.z, 1.0f };
	m_PSCbufLightColor->Update(&f);
}

void Light::Draw()
{
	Update();
	for (std::unique_ptr<Bindable>& b : m_Bindables)
	{
		b->Bind();
	}
	m_CBuf->Bind();
	m_PSCbufLightColor->Bind();

	GlobalContext::context->DrawIndexed(m_IndicesCount, 0u, 0u);
}
