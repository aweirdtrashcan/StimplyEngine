#include "Light.h"
#include "BindableIncludes.h"
#include <imgui.h>
#include "DirectX11Renderer.h"

Light::Light()
{
	std::vector<DirectX::XMFLOAT3> vertexBuffer =
	{
		{ -0.5f,  0.5f, 0.0f },
		{  0.5f, -0.5f, 0.0f },
		{ -0.5f, -0.5f, 0.0f },

		{ -0.5f,  0.5f, 0.0f },
		{  0.5f,  0.5f, 0.0f },
		{  0.5f, -0.5f, 0.0f },

		{  0.5f, -0.5f, 0.0f },
		{  0.5f,  0.5f, 0.0f },
		{  0.5f, -0.5f, 1.0f },

		{  0.5f,  0.5f, 0.0f },
		{  0.5f,  0.5f, 1.0f },
		{  0.5f, -0.5f, 1.0f },

		{  0.5f,  0.5f, 1.0f },
		{ -0.5f, -0.5f, 1.0f },
		{  0.5f, -0.5f, 1.0f },

		{  0.5f,  0.5f, 1.0f },
		{ -0.5f,  0.5f, 1.0f },
		{ -0.5f, -0.5f, 1.0f },

		{ -0.5f, -0.5f, 1.0f },
		{ -0.5f,  0.5f, 1.0f },
		{ -0.5f, -0.5f, 0.0f },

		{ -0.5f,  0.5f, 1.0f },
		{ -0.5f,  0.5f, 0.0f },
		{ -0.5f, -0.5f, 0.0f },

		{ -0.5f,  0.5f, 0.0f },
		{ -0.5f,  0.5f, 1.0f },
		{  0.5f,  0.5f, 0.0f },

		{ -0.5f,  0.5f, 1.0f },
		{  0.5f,  0.5f, 1.0f },
		{  0.5f,  0.5f, 0.0f },

		{  0.5f, -0.5f, 0.0f },
		{  0.5f, -0.5f, 1.0f },
		{ -0.5f, -0.5f, 0.0f },

		{  0.5f, -0.5f, 1.0f },
		{ -0.5f, -0.5f, 1.0f },
		{ -0.5f, -0.5f, 0.0f },
	};

	std::vector<unsigned short> indexBuffer;

	for (size_t i = 0; i < vertexBuffer.size(); i++)
	{
		indexBuffer.push_back((unsigned short)i);
	}

	m_IndicesCount = (UINT)indexBuffer.size();

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

	std::vector<DirectX::XMFLOAT4X4> m = { DirectX::XMFLOAT4X4() };

	m_CBuf = std::make_unique<Buffer<DirectX::XMFLOAT4X4>>(
		BufferType::ConstantBuffer,
		ShaderStage::VertexStage,
		m
	);

	/*m_Bindables.push_back(std::make_unique<Buffer<DirectX::XMFLOAT4>>(
		BufferType::ConstantBuffer,
		ShaderStage::VertexStage,
		f
	));*/

	for (std::unique_ptr<Bindable>& b : m_Bindables)
	{
		b->Bind();
	}
}

void Light::Update()
{
	if (ImGui::Begin("Light Pos"))
	{
		ImGui::SliderFloat("Light X", &m_LightPos.x, -180.f, 180.f);
		ImGui::SliderFloat("Light Y", &m_LightPos.y, -180.f, 180.f);
		ImGui::SliderFloat("Light Z", &m_LightPos.z, -180.f, 180.f);
		ImGui::End();
	}

	const DirectX::XMFLOAT4X4& proj = DirectX11Renderer::GetProjection();

	DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) *
		DirectX::XMMatrixTranslation(m_LightPos.x, m_LightPos.y, m_LightPos.z) *
		DirectX::XMLoadFloat4x4(&proj)
	);

	DirectX::XMStoreFloat4x4(&m_Matrix, m);

	m_CBuf->Update(&m_Matrix);
}

void Light::Draw()
{
	m_CBuf->Bind();

	GlobalContext::context->Draw(36u, 0u);
}
