#pragma once
#include "includes.h"

class GlobalContext
{
public:
	GlobalContext() = delete;
	GlobalContext(GlobalContext&&) = delete;
	GlobalContext(GlobalContext&) = delete;
	~GlobalContext() = delete;

	static void Initialize()
	{
		device = nullptr;
		context = nullptr;
		infoQueue = nullptr;
	}

	static void Shutdown()
	{
		Initialize();
	}

	static inline ID3D11Device* device;
	static inline ID3D11DeviceContext* context;
	static inline ID3D11InfoQueue* infoQueue;	
};