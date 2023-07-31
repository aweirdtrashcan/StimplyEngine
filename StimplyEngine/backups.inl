/*Vertex vertexBuffer[] =
		{
			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },

			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },

			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },

			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

			{ { -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },

			{ { -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },

			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

			{ { -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },

			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

			{ {  0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		};*/

		// create texture

		/*const Surface s = Surface::FromFile("Images\\r.png");

		D3D11_TEXTURE2D_DESC rTextDesc{};
		rTextDesc.ArraySize = 1u;
		rTextDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		rTextDesc.CPUAccessFlags = 0u;
		rTextDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rTextDesc.Height = s.GetHeight();
		rTextDesc.Width = s.GetWidth();
		rTextDesc.MipLevels = 1u;
		rTextDesc.MiscFlags = 0u;
		rTextDesc.SampleDesc.Count = 1u;
		rTextDesc.SampleDesc.Quality = 0u;
		rTextDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA rTextSrd{};
		rTextSrd.pSysMem = s.GetBufferPtr();
		rTextSrd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);
		DXERR(m_Device->CreateTexture2D(&rTextDesc, &rTextSrd, &m_RTexture), "failed to create r-texture");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = rTextDesc.Format;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DXERR(m_Device->CreateShaderResourceView(m_RTexture.Get(), &srvDesc, &m_RTextureView), "Failed to create r-texture view");

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = 5.0f;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		DXERR(m_Device->CreateSamplerState(&samplerDesc, &m_SamplerState), "Failed to create sampler state");

		m_Context->PSSetSamplers(0u, 1u, m_SamplerState.GetAddressOf());
		m_Context->PSSetShaderResources(0u, 1u, m_RTextureView.GetAddressOf());*/