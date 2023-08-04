#include "HumanModel.h"

#include "BindableIncludes.h"
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "Surface.h"
#include <imgui.h>
#include "DirectX11Renderer.h"

HumanModel::HumanModel()
{
	Assimp::Importer imp;
	const aiScene* model = imp.ReadFile("obj\\FinalBaseMesh.obj",
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);
	if (!model)
	{
		MessageBoxA(nullptr, imp.GetErrorString(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
		assert(false);
	}
	const aiMesh* mesh = model->mMeshes[0];
	std::vector<unsigned short> indices;
	std::vector<Vertex> vertices;

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		if (&mesh->mNormals[i] != nullptr)
		{
			vertices.push_back(
				{
					{ *(DirectX::XMFLOAT3*)&mesh->mVertices[i] },
					{ *(DirectX::XMFLOAT3*)&mesh->mNormals[i] }
				}
			);
		}
		else
		{
			vertices.push_back(
				{
					{ *(DirectX::XMFLOAT3*)&mesh->mVertices[i] },
					{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }
				}
			);
		}
	}

	x = 0.0f;
	y = -11.122f;
	z = 113.707f;
	yaw = DirectX::XMConvertToRadians(180.f);

	m_Bindables.push_back(std::make_unique<Buffer<Vertex>>(
		BufferType::VertexBuffer,
		ShaderStage::UnknownOrNotUsed,
		vertices
	));

	std::unique_ptr<Buffer<unsigned short>> indexBuffer = std::make_unique<Buffer<unsigned short>>(
		BufferType::IndexBuffer,
		ShaderStage::UnknownOrNotUsed,
		indices
	);
	m_IndicesCount = indexBuffer->GetIndices();

	m_Bindables.push_back(std::move(indexBuffer));

	m_Bindables.push_back(std::make_unique<PrimitiveTopology>(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	));

	std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
		{ "Normal", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
	};

	std::unique_ptr<VertexShader> vertexShader = std::make_unique<VertexShader>(
		L"./PhongVS.cso"
	);

	ID3DBlob* vsBlob = vertexShader->GetBlob().Get();

	m_Bindables.push_back(std::move(vertexShader));

	m_Bindables.push_back(std::make_unique<PixelShader>(
		L"./PhongPS.cso"
	));

	m_Bindables.push_back(std::make_unique<InputLayout>(
		ied,
		vsBlob
	));

	m_PixelShaderCBuf = std::make_unique<Buffer<LightConstantBuffer>>(
		BufferType::ConstantBuffer,
		ShaderStage::PixelStage,
		std::vector<LightConstantBuffer>{ DirectX11Renderer::GetLightConstantBuffer() }
	);

	std::vector vecMatrices = {
		matrix
	};

	m_MatrixBuffer = std::make_unique<Buffer<Matrices>>(
		BufferType::ConstantBuffer,
		ShaderStage::VertexStage,
		vecMatrices
	);
}

void HumanModel::Update()
{
	static bool bShow = true;
	ImGui::Begin("Human", &bShow);
	if (bShow)
	{
		ImGui::Text("Scaling");
		ImGui::SliderFloat("X", &scaleXYZ[0], 0.f, 10.f);
		ImGui::SliderFloat("Y", &scaleXYZ[1], 0.f, 10.f);
		ImGui::SliderFloat("Z", &scaleXYZ[2], 0.f, 10.f);

		ImGui::Text("Rotation");
		ImGui::SliderAngle("Object Roll", &roll, -180.f, 180.f);
		ImGui::SliderAngle("Object Pitch", &pitch, -180.f, 180.f);
		ImGui::SliderAngle("Object Yaw", &yaw, -180.f, 180.f);

		ImGui::Text("Position");
		ImGui::SliderFloat("Object X", &x, -30.f, 30.f);
		ImGui::SliderFloat("Object Y", &y, -30.f, 30.f);
		ImGui::SliderFloat("Object Z", &z, -30.f, 30.f * 20.f);
		
		ImGui::End();
	}

	DirectX::XMFLOAT4X4 proj = DirectX11Renderer::GetProjection();

	DirectX::XMMATRIX model = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		DirectX::XMMatrixTranslation(x, y, z) *
		DirectX::XMMatrixScaling(scaleXYZ[0], scaleXYZ[1], scaleXYZ[2]) *
		DirectX::XMLoadFloat4x4(&proj)
	);

	DirectX::XMFLOAT4X4 f;
	DirectX::XMStoreFloat4x4(&f, model);

	DirectX::XMMATRIX mvp = DirectX::XMMatrixIdentity();

	DirectX::XMStoreFloat4x4(&matrix.model, model);
	DirectX::XMStoreFloat4x4(&matrix.mvp, mvp);

	m_MatrixBuffer->Update(&matrix);
	m_PixelShaderCBuf->Update((LightConstantBuffer*)&DirectX11Renderer::GetLightConstantBuffer());
}

void HumanModel::Draw()
{
	Update();
	for (std::unique_ptr<Bindable>& bind : m_Bindables)
	{
		bind->Bind();
	}
	m_MatrixBuffer->Bind();
	m_PixelShaderCBuf->Bind();

	GlobalContext::context->DrawIndexed(m_IndicesCount, 0u, 0u);
}
