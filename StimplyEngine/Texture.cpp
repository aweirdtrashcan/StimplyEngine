#include "Texture.h"

#include "SamplerDescriptor.h"
#include "ShaderResourceView.h"

Texture::~Texture()
{
	delete m_ShaderResourceView;
}

void Texture::Bind()
{
	m_SamplerDescriptor->Bind();
	GlobalContext::context;
}

void Texture::AddSamplerDescriptor(SamplerDescriptor* samplerDescriptor)
{
	if (samplerDescriptor != nullptr)
	{
		m_SamplerDescriptor = samplerDescriptor;
	}
	else
	{
		std::stringstream ss;
		ss << "An error has occurred: \n" <<
			"Function Texture::AddSamplerDescriptor failed at line (" <<
			__LINE__ << ") at file (" << __FILE__ << ").\n" <<
			"Pointer passed as samplerDescriptor is nullptr";
		MessageBoxA(nullptr, ss.str().c_str(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}
}

void Texture::CreateShaderResourceView(DXGI_FORMAT format,
	D3D11_SRV_DIMENSION dimension,
	ID3D11Resource* resource,
	TextureType type,
	ShaderStage stage)
{
	m_ShaderResourceView = new ShaderResourceView(format, dimension, resource, type, stage);
}
