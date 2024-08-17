#pragma once

#include "defines.h"
#include "containers/list.h"
#include "vulkan_types.inl"

#include <vulkan/vulkan.h>

class VulkanBackend;
class VulkanDevice;
class VulkanImage;

class VulkanSwapchain {
public:
	VulkanSwapchain(const VulkanBackend* backend, const VulkanDevice* device, VulkanSwapchainExtent extent);
	VulkanSwapchain(const VulkanSwapchain&) = delete;
	VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
 	~VulkanSwapchain();

	bool Recreate(VulkanSwapchainExtent extent);
	bool AcquireNextImageIndex(uint64_t timeoutNanosec, VkSemaphore imageAvailableSemaphore, VkFence fence, uint32_t& outImageIndex);
	void Present(VkQueue graphicsQueue, VkQueue presentQueue, VkSemaphore renderCompleteSemaphore, uint32_t presentImageIndex);
	VulkanSwapchainExtent GetSwapchainExtent() const;

private:
	void CreateSwapchain(VulkanSwapchainExtent extent);
	void DestroySwapchain();

private:
	const VulkanBackend* m_Backend;
	const VulkanDevice* m_Device;
	VkSurfaceFormatKHR m_ImageFormat{};
	VkFormat m_DepthFormat;
	uint8_t m_MaxFramesInFlight = 0;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	uint32_t m_ImageCount = 0;
	list<VkImage> m_Images;
	VulkanImage* m_DepthImage;
	list<VkImageView> m_Views;
	uint32_t m_ImageIndex = 0;
	uint32_t m_CurrentFrame = 0;
	bool m_RecreatingSwapchain = false;
};