#include "renderer_frontend.h"

bool RendererFrontend::DrawFrame(const RenderPacket& renderPacket) {
	if (m_Backend.BeginFrame(renderPacket.deltaTime)) {
		if (!m_Backend.EndFrame(renderPacket.deltaTime)) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}