#include "vulkan_image.h"

#include "vulkan_backend.h"
#include "vulkan_device.h"

VulkanImage::VulkanImage(const VulkanBackend* backend, VkImageType imageType, uint32_t width, uint32_t height,
	VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties,
	bool createView, VkImageAspectFlags viewAspect) 
	: 
	m_Backend(backend),
	m_Width(width),
	m_Height(height) {
	
	VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = m_Width;
	createInfo.extent.height = m_Height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 4;
	createInfo.arrayLayers = 1;
	createInfo.format = format;
	createInfo.tiling = tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = usage;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateImage(m_Backend->GetVulkanDevice(), &createInfo, m_Backend->GetVulkanAllocator(), &m_Image));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_Backend->GetVulkanDevice(), m_Image, &memoryRequirements);

	uint32_t memoryTypeIndex = m_Backend->FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties);
	if (memoryTypeIndex == -1) {
		throw RendererException("Failed to find memoryTypeIndex for image");
	} 

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VK_CHECK(vkAllocateMemory(m_Backend->GetVulkanDevice(), &allocateInfo, m_Backend->GetVulkanAllocator(), &m_Memory));

	VK_CHECK(vkBindImageMemory(m_Backend->GetVulkanDevice(), m_Image, m_Memory, 0));

	VkImageViewCreateInfo viewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewCreateInfo.image = m_Image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange.aspectMask = viewAspect;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(m_Backend->GetVulkanDevice(), &viewCreateInfo, m_Backend->GetVulkanAllocator(), &m_View));	
}


VulkanImage::~VulkanImage() {
	vkDestroyImageView(m_Backend->GetVulkanDevice(), m_View, m_Backend->GetVulkanAllocator());
	vkDestroyImage(m_Backend->GetVulkanDevice(), m_Image, m_Backend->GetVulkanAllocator());
	vkFreeMemory(m_Backend->GetVulkanDevice(), m_Memory, m_Backend->GetVulkanAllocator());
}