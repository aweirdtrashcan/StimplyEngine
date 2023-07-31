#pragma once
#include "Bindable.h"

class PrimitiveTopology : public Bindable
{
public:
	PrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
		:
		m_Topology(topology)
	{}

	
	virtual void Bind() override 
	{
		m_DeviceCtx->context->IASetPrimitiveTopology(m_Topology);
	};
private:
	D3D11_PRIMITIVE_TOPOLOGY m_Topology;
};