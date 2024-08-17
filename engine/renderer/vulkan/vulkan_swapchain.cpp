#include "vulkan_swapchain.h"

#include "vulkan_backend.h"
#include "vulkan_device.h"
#include "vulkan_types.inl"
#include "core/logger.h"
#include "platform/platform.h"
#include "vulkan_image.h"

#include <algorithm>

VulkanSwapchain::VulkanSwapchain(const VulkanBackend* backend, const VulkanDevice* device, VulkanSwapchainExtent extent) 
	: 
	m_Backend(backend),
	m_Device(device) {
	Logger::Info("Creating Vulkan Swapchain");
	CreateSwapchain(extent);

	if (m_Swapchain == nullptr) {
		throw RendererException("Failed to create the swapchain");
	}


}

VulkanSwapchain::~VulkanSwapchain() {
	Logger::Info("Destroying Vulkan Swapchain");
	DestroySwapchain();
}


bool VulkanSwapchain::Recreate(VulkanSwapchainExtent extent) {
	m_RecreatingSwapchain = true;
	
	DestroySwapchain();
	CreateSwapchain(extent);
	
	m_RecreatingSwapchain = false;

	return m_Swapchain != nullptr;
}

bool VulkanSwapchain::AcquireNextImageIndex(
	uint64_t timeoutNanosec, VkSemaphore imageAvailableSemaphore, 
	VkFence fence, uint32_t& outImageIndex) {
	
	VkResult result = vkAcquireNextImageKHR(
		*m_Device, 
		m_Swapchain, 
		timeoutNanosec, 
		imageAvailableSemaphore, 
		fence, 
		&outImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		Recreate(GetSwapchainExtent());
		return false;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw RendererException("Failed to acquire swapchain image with error: %s", string_VkResult(result));
	}

	return true;
}

void VulkanSwapchain::Present(
	VkQueue graphicsQueue, VkQueue presentQueue, 
	VkSemaphore renderCompleteSemaphore, uint32_t presentImageIndex) {

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pImageIndices = &presentImageIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		Recreate(GetSwapchainExtent());
	} else if (result != VK_SUCCESS) {
		throw RendererException("Failed to present swapchain image with error: %s", string_VkResult(result));
	}
}

VulkanSwapchainExtent VulkanSwapchain::GetSwapchainExtent() const {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		*m_Device, 
		m_Backend->GetVulkanSurface(), 
		&surfaceCapabilities);

	VulkanSwapchainExtent extent;
	extent.width = surfaceCapabilities.currentExtent.width;
	extent.height = surfaceCapabilities.currentExtent.height;

	return extent;
}

void VulkanSwapchain::CreateSwapchain(VulkanSwapchainExtent extent) {
	if (extent.width == 0 || extent.height == 0) {
		extent = GetSwapchainExtent();
		if (extent.width == 0 || extent.height == 0) {
			DestroySwapchain();
			return;
		}
	}

	m_MaxFramesInFlight = 2;
	
	const VulkanSwapchainSupportInfo& swapChainSupport = m_Device->GetSwapchainSupport();

	// Choose a surface format.

	bool found = false;
	for (VkSurfaceFormatKHR format : swapChainSupport.formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			m_ImageFormat = format;
			found = true;
			break;
		}
	}

	if (!found) {
		m_ImageFormat = swapChainSupport.formats[0];
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (VkPresentModeKHR supportedMode : swapChainSupport.presentModes) {
		if (supportedMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = supportedMode;
			break;
		}
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		*m_Device, 
		m_Backend->GetVulkanSurface(), 
		&surfaceCapabilities);

	if (extent.width > surfaceCapabilities.maxImageExtent.width) {
		Logger::Info(
			"Width of %u is bigger than the maximum supported by the GPU, decreasing to %u...", 
			extent.width, 
			surfaceCapabilities.maxImageExtent.width);
		extent.width = surfaceCapabilities.maxImageExtent.width;
	}
	if (extent.height > surfaceCapabilities.maxImageExtent.height) {
		Logger::Info(
			"Height of %u is bigger than the maximum supported by the GPU, decreasing to %u...", 
			extent.height, 
			surfaceCapabilities.maxImageExtent.height);
		extent.height = surfaceCapabilities.maxImageExtent.height;
	}
	if (extent.width < surfaceCapabilities.minImageExtent.width) {
		Logger::Info(
			"Width of %u is smaller than the minimum supported by the GPU, increasing to %u...", 
			extent.width, 
			surfaceCapabilities.minImageExtent.width);
		extent.width = surfaceCapabilities.minImageExtent.width;
	}
	if (extent.height < surfaceCapabilities.maxImageExtent.height) {
		Logger::Info(
			"Height of %u is smaller than the minimum supported by the GPU, increasing to %u...", 
			extent.height, 
			surfaceCapabilities.minImageExtent.height);
		extent.height = surfaceCapabilities.minImageExtent.height;
	}

	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	imageCount = std::clamp(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = m_Backend->GetVulkanSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = m_ImageFormat.format;
	createInfo.imageColorSpace = m_ImageFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Sharing mode
	VulkanQueueFamilyIndices queueFamilyIndices = m_Device->GetQueueFamilyIndices();
	uint32_t indices[] = { queueFamilyIndices.graphicsQueueFamilyIndex, queueFamilyIndices.transferQueueFamilyIndex };

	if (queueFamilyIndices.graphicsQueueFamilyIndex != queueFamilyIndices.transferQueueFamilyIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	
	VK_CHECK(vkCreateSwapchainKHR(
		*m_Device, 
		&createInfo, 
		m_Backend->GetVulkanAllocator(), 
		&m_Swapchain));

	VK_CHECK(vkGetSwapchainImagesKHR(
		*m_Device, 
		m_Swapchain, 
		&m_ImageCount, 
		nullptr));

	m_Images.resize(m_ImageCount);
	m_Views.resize(m_ImageCount);

	VK_CHECK(vkGetSwapchainImagesKHR(
		*m_Device, 
		m_Swapchain, 
		&m_ImageCount, 
		m_Images.data()));

	for (uint64_t i = 0; i < m_Views.size(); i++) {
		VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = m_Images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_ImageFormat.format;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		
		VK_CHECK(vkCreateImageView(*m_Device, &createInfo, m_Backend->GetVulkanAllocator(), &m_Views[i]));
	}

	m_CurrentFrame = 0;
	m_ImageIndex = 0;

	m_DepthFormat = m_Device->DetectDepthFormat(false);

	VkImageAspectFlags depthAspect;

	if (m_DepthFormat == VK_FORMAT_D32_SFLOAT) {
		depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	} else if (m_DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT || m_DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	m_DepthImage = Platform::Construct<VulkanImage>(
		m_Backend,
		VK_IMAGE_TYPE_2D,
		createInfo.imageExtent.width,
		createInfo.imageExtent.height,
		m_DepthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		depthAspect
	);

}

void VulkanSwapchain::DestroySwapchain() {
	for (VkImageView view : m_Views) {
		vkDestroyImageView(*m_Device, view, m_Backend->GetVulkanAllocator());
	}
	m_Images.resize(0);
	m_Views.resize(0);
	vkDestroySwapchainKHR(*m_Device, m_Swapchain, m_Backend->GetVulkanAllocator());
	m_Swapchain = VK_NULL_HANDLE;
	Platform::Destroy(m_DepthImage);
	m_DepthImage = nullptr;
}

