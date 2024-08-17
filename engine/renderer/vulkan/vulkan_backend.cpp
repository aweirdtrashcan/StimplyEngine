#include "vulkan_backend.h"

#include "core/logger.h"
#include "core/string.h"
#include "platform/platform.h"

#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "window/window.h"

VulkanBackend::VulkanBackend(const char* applicationName, const Window& window) 
	:
	RendererBackend(applicationName, window) {
	Logger::Info("Initializing Vulkan backend");

	VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	app_info.apiVersion = VK_API_VERSION_1_2;
	app_info.pApplicationName = applicationName;
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	app_info.pEngineName = "Stimply Engine";
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

	auto debugUtilsCreateInfo = GetDebugMessengerCreateInfo();

	VkInstanceCreateInfo instance_create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instance_create_info.pNext = &debugUtilsCreateInfo;
	instance_create_info.pApplicationInfo = &app_info;

	list<const char*> requiredExtensions = Platform::GetRequiredExtensionNames(m_Window);

#ifdef DEBUG
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	requiredExtensions.push_back("VK_EXT_debug_utils");

	Logger::Debug("Required extensions:");
	for (const char* requiredExtension : requiredExtensions) {
		Logger::Debug("\t%s", requiredExtension);
	}

	// Validation layers.
	list<const char*> requiredLayers = GetRequiredInstanceLayers();
	
	instance_create_info.enabledLayerCount = requiredLayers.size_u32();
	instance_create_info.ppEnabledLayerNames = requiredLayers.data();

#endif

	instance_create_info.enabledExtensionCount = requiredExtensions.size_u32();
	instance_create_info.ppEnabledExtensionNames = requiredExtensions.data();

	VK_CHECK(vkCreateInstance(&instance_create_info, m_Allocator, &m_Instance));

#ifdef DEBUG
	// create debug utils messenger.
	PFN_vkCreateDebugUtilsMessengerEXT func = 
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");

	if (func) {
		func(m_Instance, &debugUtilsCreateInfo, m_Allocator, &m_Messenger);
	}
	Logger::Debug("Vulkan debugger created");
#endif

	m_Surface = (VkSurfaceKHR)Platform::CreateVulkanSurface(&m_Window, m_Instance);

	if (!m_Surface) {
		throw RendererException("Failed to create vulkan surface");
	}

	m_Device = Platform::Construct<VulkanDevice>(this);
	uint32_t width, height;
	m_Window.GetDimensions(&width, &height);
	m_Swapchain = Platform::Construct<VulkanSwapchain>(this, m_Device, VulkanSwapchainExtent{ width, height });
}

VulkanBackend::~VulkanBackend() {
	Logger::Info("Destroying Vulkan backend");
	Platform::Destroy(m_Swapchain);
	Platform::Destroy(m_Device);
	vkDestroySurfaceKHR(m_Instance, m_Surface, m_Allocator);
#ifdef DEBUG
	// destroy debug utils
	PFN_vkDestroyDebugUtilsMessengerEXT func = 
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func) {
		func(m_Instance, m_Messenger, m_Allocator);
	}
	Logger::Debug("Vulkan debugger destroyed");
#endif
	vkDestroyInstance(m_Instance, m_Allocator);
}

void VulkanBackend::Resized(uint32_t width, uint32_t height) {

}

bool VulkanBackend::BeginFrame(float deltaTime) {
	return true;
}

bool VulkanBackend::EndFrame(float deltaTime) {
	m_NumFramesRendered++;
	return true;
}

void VulkanBackend::WaitDeviceIdle() {

}

uint32_t VulkanBackend::FindMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperties) const {
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(*m_Device, &deviceMemoryProperties);

	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
		if (memoryTypeBits & (1 << i) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties) {
			return i;
		}
	}

	return -1;
}

VkBool32 VulkanBackend::PrintDebugLayer(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
			Logger::Fatal("%s", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
			Logger::Warning("%s", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
			Logger::Debug("%s", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
			Logger::Info("%s", pCallbackData->pMessage);
			break;
		}
		default: {
			Logger::Info("%s", pCallbackData->pMessage);
			break;
		}
	}

	return true;
}

list<const char*> VulkanBackend::GetRequiredInstanceLayers() {
	list<const char*> requiredLayers;

	requiredLayers.push_back("VK_LAYER_KHRONOS_validation");

	uint32_t supportedLayerCount = 0;
	list<VkLayerProperties> supportedLayers;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr)); 
	supportedLayers.resize(supportedLayerCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data()));

	for (const char* requiredLayer : requiredLayers) {
		Logger::Info("Searching for layer: %s", requiredLayer);
		bool found = false;

		for (const VkLayerProperties& supportedLayer : supportedLayers) {
			if (String::StringEqual(requiredLayer, supportedLayer.layerName)) {
				found = true;
				Logger::Info("%s Found", requiredLayer);	
				break;
			}
		}

		if (!found) {
			Logger::Fatal("Required instance layer %s is not supported!", requiredLayer);
			uint32_t foundIndex = requiredLayers.find_index(requiredLayer);

			if (foundIndex != -1) {
				requiredLayers.remove_at(foundIndex);
			}
		}
	}

	return requiredLayers;
}

VkDebugUtilsMessengerCreateInfoEXT VulkanBackend::GetDebugMessengerCreateInfo() {
	VkDebugUtilsMessengerCreateInfoEXT create_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
#ifdef DEBUG
	create_info.pfnUserCallback = VulkanBackend::PrintDebugLayer;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
							  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#endif
	return create_info;
}
