#pragma once

#include "renderer/renderer_backend.h"
#include "renderer/renderer_types.inl"

class RendererFrontend {
public:
	RendererFrontend(RendererBackend& backend)
		:
		m_Backend(backend)
	{}
	~RendererFrontend() = default;

	AINLINE void Resized(uint32_t width, uint32_t height) { m_Backend.Resized(width, height); }
	bool DrawFrame(const RenderPacket& renderPacket);
    AINLINE void WaitDeviceIdle() { m_Backend.WaitDeviceIdle(); }

private:
	RendererBackend& m_Backend;
};