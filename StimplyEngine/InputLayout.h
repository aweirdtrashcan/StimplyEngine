#pragma once
#include "Bindable.h"

class InputLayout : public Bindable
{
public:
	InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> ied, ID3DBlob* vertShaderBytecode)
	{
		DXERR(GlobalContext::device->CreateInputLayout(
			ied.data(), 
			(UINT)ied.size(), 
			vertShaderBytecode->GetBufferPointer(), 
			vertShaderBytecode->GetBufferSize(),
			&m_InputLayout),
			"Failed to create Input Layout");

	}

	virtual void Bind() override
	{
		GlobalContext::context->IASetInputLayout(m_InputLayout.Get());
	}

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
};