#pragma once
#include "Bindable.h"

class VertexShader : public Bindable
{
public:
	VertexShader(const wchar_t* shaderPath)
	{
		DXERR(D3DReadFileToBlob(shaderPath, &m_Blob), "Failed to compile Vertex Shader");

		DXERR(GlobalContext::device->CreateVertexShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_VertexShader),
			"Failed to create Pixel Shader");
	}

	virtual void Bind() override
	{
		GlobalContext::context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_Blob; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_Blob;
};
