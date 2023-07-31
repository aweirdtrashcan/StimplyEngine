#pragma once
#include "Bindable.h"

class SamplerDescriptor : public Bindable
{
public:
	SamplerDescriptor(D3D11_TEXTURE_ADDRESS_MODE addressMode, FLOAT* borderColor, const DeviceContext* deviceCtx)
	{
		InfoMan();
		D3D11_SAMPLER_DESC desc{};
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;
		if (borderColor != nullptr)
		{
			for (int i = 0; i < 4; i++)
			{
				std::stringstream ss;
				ss << "Border color index" << i << "is null";
				assert(borderColor[i] && ss.str().c_str());
			}
			desc.BorderColor[0] = borderColor[0];
			desc.BorderColor[1] = borderColor[1];
			desc.BorderColor[2] = borderColor[2];
			desc.BorderColor[3] = borderColor[3];
		}
		DXERR(m_DeviceCtx->device->CreateSamplerState(&desc, &m_SamplerState),
			"Failed to create SamplerState");

	}

	virtual void Bind() override
	{
		m_DeviceCtx->context->VSSetSamplers(0u, 1u, m_SamplerState.GetAddressOf());
	}
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;
};
