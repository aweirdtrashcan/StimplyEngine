#pragma once
#include "Bindable.h"

class PixelShader : public Bindable
{
public:
	PixelShader(const wchar_t* shaderPath)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		DXERR(D3DReadFileToBlob(shaderPath, &blob), "Failed to compile Pixel Shader");

		DXERR(GlobalContext::device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_PixelShader),
			"Failed to create Pixel Shader");
	}

	virtual void Bind() override
	{
		GlobalContext::context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
};
