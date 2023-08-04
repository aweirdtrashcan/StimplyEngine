#pragma once
#include "Bindable.h"

template<typename T>
class Buffer : public Bindable
{
public:
	Buffer(BufferType type, ShaderStage stage, std::vector<T> data)
		:
		m_Type(type),
		m_Stage(stage)
	{
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(T) * (UINT)data.size();
		bd.StructureByteStride = sizeof(T);

		switch (m_Type)
		{
		case BufferType::VertexBuffer:
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_IMMUTABLE;
			break;
		case BufferType::IndexBuffer:
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.Usage = D3D11_USAGE_IMMUTABLE;
			m_IndexCount = (UINT)data.size();
			break;
		case BufferType::ConstantBuffer:
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			break;
		}

		bd.CPUAccessFlags = m_Type == BufferType::ConstantBuffer ? D3D11_CPU_ACCESS_WRITE : 0u;
		bd.MiscFlags = 0u;
		D3D11_SUBRESOURCE_DATA srd{};
		srd.pSysMem = data.data();

		DXERR(GlobalContext::device->CreateBuffer(&bd, &srd, &m_Buffer), "Failed to create Buffer");
	}
	
	virtual void Bind() override
	{
		UINT offset = 0;
		UINT stride = 0;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		switch (m_Type)
		{
		case BufferType::VertexBuffer:
			offset = 0;
			stride = sizeof(T);
			GlobalContext::context->IASetVertexBuffers(0u, 1u, m_Buffer.GetAddressOf(), &stride, &offset);
			break;
		case BufferType::IndexBuffer:
			offset = 0;
			if (sizeof(T) == 4)
			{
				format = DXGI_FORMAT_R32_UINT;
			} 
			else if (sizeof(T) == 2)
			{
				format = DXGI_FORMAT_R16_UINT;
			}
			else
			{
				MessageBoxA(nullptr, "Failed to find a suitable IndexBuffer Type. Either 16-bits or 32-bits unsigned integer", "ERROR", MB_OK | MB_ICONEXCLAMATION);
				__debugbreak();
			}
			GlobalContext::context->IASetIndexBuffer(m_Buffer.Get(), format, 0);
			break;
		case BufferType::ConstantBuffer:
			switch (m_Stage)
			{
			case ShaderStage::VertexStage:
				GlobalContext::context->VSSetConstantBuffers(0u, 1u, m_Buffer.GetAddressOf());
				break;
			case ShaderStage::PixelStage:
				GlobalContext::context->PSSetConstantBuffers(0u, 1u, m_Buffer.GetAddressOf());
				break;
			default:
				throw std::runtime_error("Failed to select shader stage");
			} break;
		default:
			MessageBoxA(nullptr, "Failed to select what is the BufferType", "ERROR", MB_OK | MB_ICONEXCLAMATION);
			__debugbreak();
			break;
		}
	}

	void Update(T* data)
	{
		D3D11_MAPPED_SUBRESOURCE msr{};
		DXERR(GlobalContext::context->Map(m_Buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr),
			"Failed to bind buffer");
		memcpy(msr.pData, data, sizeof(T));
		GlobalContext::context->Unmap(m_Buffer.Get(), 0u);
	}

	const UINT GetIndices() const { return m_IndexCount; }
private:
	BufferType m_Type;
	ShaderStage m_Stage;
	UINT m_IndexCount = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
};

