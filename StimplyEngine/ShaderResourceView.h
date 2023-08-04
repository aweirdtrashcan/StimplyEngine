#pragma once
#include "Bindable.h"

class ShaderResourceView : public Bindable
{
public:
	ShaderResourceView(
		DXGI_FORMAT format, 
		D3D11_SRV_DIMENSION dimension, 
		ID3D11Resource* resource, 
		TextureType type, 
		ShaderStage stage)
		:
		m_Stage(stage)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = format;
		if (type == TextureType::Texture2D)
		{
			desc.Texture2D.MipLevels = 1u;
			desc.Texture2D.MostDetailedMip = 0u;
		}
		desc.ViewDimension = dimension;
		DXERR(GlobalContext::device->CreateShaderResourceView(resource, &desc, &m_SRV), 
			"Failed to create Shader Resource View");
	}

	virtual void Bind() override
	{
		if (m_Stage == ShaderStage::VertexStage)
		{
			GlobalContext::context->VSSetShaderResources(0u, 1u, m_SRV.GetAddressOf());
		} 
		if (m_Stage == ShaderStage::PixelStage)
		{
			GlobalContext::context->PSSetShaderResources(0u, 1u, m_SRV.GetAddressOf());
		}
	}

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	const ShaderStage m_Stage;
};
