#pragma once

#include "vulkan_types.inl"

#include <vulkan/vulkan.h>

enum class VulkanRenderPassState {
	Ready,
	Recording,
	InRenderPass,
	RecordingEnded,
	Submitted,
	NotAllocated
};

class VulkanBackend;
class VulkanFramebuffer;
class VulkanCommandBuffer;

class VulkanRenderPass {
public:
	VulkanRenderPass(const VulkanBackend* backend, VulkanRenderPassRect dimensions, float depth, uint32_t stencil);
	~VulkanRenderPass();

	void Begin(const VulkanCommandBuffer* commandBuffer, const VulkanFramebuffer* framebuffer);
	void End(const VulkanCommandBuffer* commandBuffer);
private:
	const VulkanBackend* m_Backend;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	VulkanRenderPassRect m_RenderPassRect;
	VulkanRenderPassState m_State = VulkanRenderPassState::NotAllocated;
};