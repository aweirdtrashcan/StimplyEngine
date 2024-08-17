#include "vulkan_device.h"

#include "core/logger.h"
#include "core/string.h"
#include "vulkan_backend.h"

#if defined (PLATFORM_LINUX) || defined(PLATFORM_MAC)
#include <alloca.h>
#elif defined (PLATFORM_WINDOWS)
#include <malloc.h>
#ifdef alloca
#undef alloca
#endif
#define alloca(size) _alloca(size)
#endif

VulkanDevice::VulkanDevice(const VulkanBackend* backend) : backend(backend) {
	Logger::Info("Creating Vulkan Device");
	ChoosePhysicalDevice();
	CreateLogicalDevice();
}

VulkanDevice::~VulkanDevice() {
	Logger::Info("Destroying Vulkan Device");
	vkDestroyDevice(m_LogicalDevice, backend->GetVulkanAllocator());
	m_LogicalDevice = VK_NULL_HANDLE;
	m_TransferQueue = VK_NULL_HANDLE;
	m_PresentQueue = VK_NULL_HANDLE;
	m_GraphicsQueue = VK_NULL_HANDLE;
}

VkFormat VulkanDevice::DetectDepthFormat(bool useStencil) const {
	VkFormat* desiredFormats;
	uint32_t formatCount = 0;

	if (useStencil) {
		formatCount = 2;
		desiredFormats = (VkFormat*)alloca(sizeof(VkFormat) * formatCount);
		desiredFormats[0] = VK_FORMAT_D24_UNORM_S8_UINT;
		desiredFormats[1] = VK_FORMAT_D32_SFLOAT_S8_UINT;
 	} else {
		formatCount = 1;
		desiredFormats = (VkFormat*)alloca(sizeof(VkFormat));
		desiredFormats[0] = VK_FORMAT_D32_SFLOAT;
	}

	uint32_t flag = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (uint32_t i = 0; i < formatCount; i++) {
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, desiredFormats[i], &formatProperties);

		if ((formatProperties.linearTilingFeatures & flag) == flag) {
			return desiredFormats[i];
		} else if ((formatProperties.optimalTilingFeatures & flag) == flag) {
			return desiredFormats[i];
		}
	}

	throw RendererException("Failed to find a Depth Format for this device");
}

void VulkanDevice::ChoosePhysicalDevice() {
	list<VkPhysicalDevice> physicalDevices;
	uint32_t physicalDeviceCount = 0;

	VK_CHECK(vkEnumeratePhysicalDevices(backend->GetVulkanInstance(), &physicalDeviceCount, nullptr));
	if (physicalDeviceCount == 0) {
		throw RendererException("No compatible physical devices were found");
	}

	physicalDevices.resize(physicalDeviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(backend->GetVulkanInstance(), &physicalDeviceCount, physicalDevices.data()));

	for (VkPhysicalDevice physicalDevice : physicalDevices) {
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceMemoryProperties memory;
		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memory);
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);

		VulkanPhysicalDeviceRequirements requirements;
		requirements.graphics = true;
		requirements.present = true;
		requirements.transfer = true;
		requirements.compute = false;
		requirements.deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		requirements.samplerAnisotropy = true;
		requirements.discreteGpu = true;

		VulkanQueueFamilyIndices queueFamilyIndices;
		VulkanSwapchainSupportInfo swapchainSupport;

		if (!PhysicalDeviceMeetsRequirements( 
			physicalDevice, 
			backend->GetVulkanSurface(), 
			properties, 
			features, 
			requirements, 
			queueFamilyIndices, 
			swapchainSupport)) {
			Logger::Info("%s does not meet physical device requirements", properties.deviceName);
			continue;
		}

		m_PhysicalDevice = physicalDevice;
		m_DeviceProperties = properties;
		m_DeviceMemoryProperties = memory;
		m_DeviceFeatures = features;
		m_QueueFamilyIndices = queueFamilyIndices;
		m_SwapchainSupport = swapchainSupport;

		Logger::Info("Selected Device %s", properties.deviceName);
		Logger::Info("Graphics Queue Index: %d", m_QueueFamilyIndices.graphicsQueueFamilyIndex);
		Logger::Info("Transfer Queue Index: %d", m_QueueFamilyIndices.transferQueueFamilyIndex);
		Logger::Info("Compute Queue Index: %d", m_QueueFamilyIndices.computeQueueFamilyIndex);
		Logger::Info("Present Queue Index: %d", m_QueueFamilyIndices.presentQueueFamilyIndex);
		break;
	}
	
	if (m_PhysicalDevice == VK_NULL_HANDLE) {
		throw RendererException("Failed to find a suitable vulkan physical device");
	}
}

void VulkanDevice::CreateLogicalDevice() {
	bool presentSharesGraphicsQueue = m_QueueFamilyIndices.presentQueueFamilyIndex == m_QueueFamilyIndices.graphicsQueueFamilyIndex;
	bool hasTransfer = m_QueueFamilyIndices.transferQueueFamilyIndex != -1;
	bool transferSharesGraphicsQueue = 
		m_QueueFamilyIndices.transferQueueFamilyIndex == m_QueueFamilyIndices.graphicsQueueFamilyIndex;

	list<uint32_t> indices;
	indices.push_back(m_QueueFamilyIndices.graphicsQueueFamilyIndex);

	if (!presentSharesGraphicsQueue) [[unlikely]] {
		indices.push_back(m_QueueFamilyIndices.presentQueueFamilyIndex);
	}

	if (!transferSharesGraphicsQueue && hasTransfer) {
		indices.push_back(m_QueueFamilyIndices.transferQueueFamilyIndex);
	}

	list<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.resize(indices.size());

	for (uint32_t i = 0; i < indices.size_u32(); i++) {
		queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[i].queueFamilyIndex = indices[i];
		queueCreateInfos[i].queueCount = 1;
		// bool shouldReuseGraphicsToPresentQueue = indices[i] == m_QueueFamilyIndices.graphicsQueueFamilyIndex && presentSharesGraphicsQueue;
		// if (shouldReuseGraphicsToPresentQueue) {
		// 	queueCreateInfos[i].queueCount = 2;
		// }
		float priority = 1.0f;
		queueCreateInfos[i].pQueuePriorities = &priority;
	} 

	// Request device features.
	VkPhysicalDeviceFeatures requestFeatures{};
	requestFeatures.samplerAnisotropy = VK_TRUE;

	list<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = queueCreateInfos.size_u32();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = deviceExtensions.size_u32();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pEnabledFeatures = &requestFeatures;

	VK_CHECK(vkCreateDevice(
		m_PhysicalDevice, 
		&createInfo, 
		backend->GetVulkanAllocator(), 
		&m_LogicalDevice));

	vkGetDeviceQueue(
		m_LogicalDevice, 
		m_QueueFamilyIndices.graphicsQueueFamilyIndex, 
		0, 
		&m_GraphicsQueue);
	
	if (presentSharesGraphicsQueue) {
		vkGetDeviceQueue(
			m_LogicalDevice, 
			m_QueueFamilyIndices.graphicsQueueFamilyIndex, 
			0, 
			&m_PresentQueue);
	} else {
		vkGetDeviceQueue(
			m_LogicalDevice, 
			m_QueueFamilyIndices.presentQueueFamilyIndex, 
			0, 
			&m_PresentQueue);
	}

	if (hasTransfer) {
		vkGetDeviceQueue(
			m_LogicalDevice, 
			m_QueueFamilyIndices.transferQueueFamilyIndex, 
			0, 
			&m_TransferQueue);
	}
}

VulkanSwapchainSupportInfo VulkanDevice::QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {
	VulkanSwapchainSupportInfo supportInfo;

	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice,
		surface,
		&supportInfo.surfaceCapabilities
	));

	uint32_t formatCount = 0;
	uint32_t presentModeCount = 0;

	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice, 
		surface, 
		&formatCount, 
		nullptr));

	supportInfo.formats.resize(formatCount);

	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice, 
		surface, 
		&formatCount, 
		supportInfo.formats.data()));

	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice, 
		surface,
		&presentModeCount, 
		nullptr));

	supportInfo.presentModes.resize(presentModeCount);

	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice, 
		surface,
		&presentModeCount, 
		supportInfo.presentModes.data()));

	return supportInfo;
}

bool VulkanDevice::PhysicalDeviceMeetsRequirements(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	const VkPhysicalDeviceProperties& properties,
	const VkPhysicalDeviceFeatures& features,
	const VulkanPhysicalDeviceRequirements& requirements,
	VulkanQueueFamilyIndices& outQueueFamilyIndices,
	VulkanSwapchainSupportInfo& outSwapchainSupportInfo) {
	
	outQueueFamilyIndices = VulkanQueueFamilyIndices();

	if (requirements.discreteGpu && properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		Logger::Info("%s is not a discrete GPU!", properties.deviceName);
		return false;
	}

	if (requirements.samplerAnisotropy && !features.samplerAnisotropy) {
		Logger::Info("%s does not support Sampler Anisotropy!", properties.deviceName);
		return false;
	}

	uint32_t queueFamilyCount = 0;
	list<VkQueueFamilyProperties> queueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
	
	if (queueFamilyCount == 0) {
		Logger::Warning("%s does not have any queue families. Does this device really supports Vulkan?");
		return false;
	}

	uint8_t minTransferScore = 255;
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		uint8_t currentTransferScore = 0;

		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			outQueueFamilyIndices.graphicsQueueFamilyIndex = i;
			currentTransferScore++;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			outQueueFamilyIndices.computeQueueFamilyIndex = i;
			currentTransferScore++;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			if (currentTransferScore < minTransferScore) {
				outQueueFamilyIndices.transferQueueFamilyIndex = i;
				minTransferScore = currentTransferScore;
			}
		}

		VkBool32 isPresent = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &isPresent);

		if (isPresent == VK_TRUE) {
			outQueueFamilyIndices.presentQueueFamilyIndex = i;
		}
	}

	const char* format = 
			"Device: %s Supported Queue Families:\n"
			"\t** Graphics: %s\n"
			"\t** Present: %s\n"
			"\t** Compute: %s\n"
			"\t** Transfer: %s\n";
			
	Logger::Debug(
		format,
		properties.deviceName,
		outQueueFamilyIndices.graphicsQueueFamilyIndex != -1 ? "true" : "false",
		outQueueFamilyIndices.presentQueueFamilyIndex != -1 ? "true" : "false",
		outQueueFamilyIndices.computeQueueFamilyIndex != -1 ? "true" : "false",
		outQueueFamilyIndices.transferQueueFamilyIndex != -1 ? "true" : "false"
	);

	if (requirements.graphics && outQueueFamilyIndices.graphicsQueueFamilyIndex == -1) {
		Logger::Warning("Graphics Queue required but no graphics queue index was found");
		return false;
	}

	if (requirements.compute && outQueueFamilyIndices.computeQueueFamilyIndex == -1) {
		Logger::Warning("Compute Queue required but no compute queue index was found");
		return false;
	}

	if (requirements.present && outQueueFamilyIndices.presentQueueFamilyIndex == -1) {
		Logger::Warning("Present Queue required but no present queue index was found");
		return false;
	}

	if (requirements.transfer && outQueueFamilyIndices.transferQueueFamilyIndex == -1) {
		Logger::Warning("Transfer Queue required but no transfer queue index was found");
		return false;
	}

	outSwapchainSupportInfo = QuerySwapchainSupport(physicalDevice, surface);

	if (outSwapchainSupportInfo.formats.is_empty() || outSwapchainSupportInfo.presentModes.is_empty()) {
		Logger::Info("Required formats or present modes are not supported by %s", properties.deviceName);
		return false;
	}

	if (!requirements.deviceExtensionNames.is_empty()) {
		list<VkExtensionProperties> supportedExtensions;
		uint32_t supportedExtensionCount = 0;

		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			physicalDevice, 
			nullptr, 
			&supportedExtensionCount, 
			nullptr));
		
		supportedExtensions.resize(supportedExtensionCount);

		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			physicalDevice, 
			nullptr, 
			&supportedExtensionCount, 
			supportedExtensions.data()));

		for (const char* requiredExtension : requirements.deviceExtensionNames) {
			bool found = false;
			for (const VkExtensionProperties& supportedExtension : supportedExtensions) {
				if (String::StringEqual(requiredExtension, supportedExtension.extensionName)) {
					found = true;
					break;
				}
			}

			if (!found) {
				Logger::Info("Required extension %s was not found in device %s, skipping...", requiredExtension, properties.deviceName);
				return false;
			}
		}
	}

	return true;
}