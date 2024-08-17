#pragma once

#include "defines.h"
#include <vulkan/vulkan.h>

class VulkanBackend;

class VulkanImage {
public:
	VulkanImage(const VulkanBackend* backend, VkImageType imageType, uint32_t width, uint32_t height,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties,
		bool createView, VkImageAspectFlags viewAspect);
	VulkanImage(const VulkanImage&) = delete;
	VulkanImage& operator=(const VulkanImage&) = delete;
	~VulkanImage();

	AINLINE operator VkImage() const { return m_Image; } 
	AINLINE operator VkImageView() const { return m_View; }

private:
	const VulkanBackend* m_Backend;
	VkImage m_Image = VK_NULL_HANDLE;
	VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	VkImageView m_View = VK_NULL_HANDLE;
	uint32_t m_Width;
	uint32_t m_Height;
};