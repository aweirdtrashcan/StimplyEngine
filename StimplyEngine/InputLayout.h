#pragma once
#include "Bindable.h"

class InputLayout : public Bindable
{
public:
	InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> ied, ID3DBlob* vertShaderBytecode, const DeviceContext* deviceCtx)
	{
		InfoMan();
		DXERR(m_DeviceCtx->device->CreateInputLayout(
			ied.data(), 
			(UINT)ied.size(), 
			vertShaderBytecode->GetBufferPointer(), 
			vertShaderBytecode->GetBufferSize(),
			&m_InputLayout),
			"Failed to create Input Layout");

	}

	virtual void Bind() override
	{
		m_DeviceCtx->context->IASetInputLayout(m_InputLayout.Get());
	}

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
};