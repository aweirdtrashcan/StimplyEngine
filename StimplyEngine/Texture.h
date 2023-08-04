#pragma once
#include "Bindable.h"
#include "Surface.h"

class SamplerDescriptor;
class ShaderResourceView;

class Texture : public Bindable
{
public:
	Texture(TextureType type, const Surface* surface, ShaderStage stage)
		:
		m_Type(type)
	{
		if (m_Type == TextureType::Texture2D)
		{
			D3D11_TEXTURE2D_DESC tex2DDesc{};
			tex2DDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			tex2DDesc.ArraySize = 0u;
			tex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tex2DDesc.CPUAccessFlags = 0u;
			tex2DDesc.MipLevels = 1u;
			tex2DDesc.MiscFlags = 0u;
			tex2DDesc.SampleDesc.Count = 1u;
			tex2DDesc.SampleDesc.Quality = 0u;

			D3D11_SUBRESOURCE_DATA srd{};
			
			if (surface != nullptr)
			{
				tex2DDesc.Height = surface->GetHeight();
				tex2DDesc.Width = surface->GetWidth();
				tex2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
				srd.pSysMem = surface->GetBufferPtr();
				srd.SysMemPitch = surface->GetWidth() * sizeof(Surface::Color);
			}
			else
			{
				tex2DDesc.Usage = D3D11_USAGE_DYNAMIC;
			}
			DXERR(GlobalContext::device->CreateTexture2D(&tex2DDesc, &srd, &m_Texture2D), 
				"Failed to create 2D Texture");
			CreateShaderResourceView(
				tex2DDesc.Format,
				D3D11_SRV_DIMENSION_TEXTURE2D,
				m_Texture2D.Get(),
				m_Type,
				stage);
		}
		else
		{
			D3D11_TEXTURE3D_DESC tex3DDesc{};
		}

		
	}

	virtual ~Texture();
	virtual void Bind() override;
	void AddSamplerDescriptor(SamplerDescriptor* samplerDescriptor);

private:
	void CreateShaderResourceView(DXGI_FORMAT format,
		D3D11_SRV_DIMENSION dimension,
		ID3D11Resource* resource,
		TextureType type,
		ShaderStage stage);

	TextureType m_Type;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture2D;
	Microsoft::WRL::ComPtr<ID3D11Texture3D> m_Texture3D;
	SamplerDescriptor* m_SamplerDescriptor;
	ShaderResourceView* m_ShaderResourceView;
};