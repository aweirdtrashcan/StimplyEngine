#include "vulkan_renderpass.h"

VulkanRenderPass::VulkanRenderPass(const VulkanBackend* backend, VulkanRenderPassRect dimensions, float depth, uint32_t stencil) 
	:
	m_Backend(backend),
	m_RenderPassRect(dimensions) {
	
}

VulkanRenderPass::~VulkanRenderPass() {

}
