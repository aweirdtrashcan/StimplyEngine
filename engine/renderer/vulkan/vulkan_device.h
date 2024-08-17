#pragma once

#include "defines.h"
#include "containers/list.h"

#include <vulkan/vulkan.h>

class VulkanBackend;

struct VulkanQueueFamilyIndices {
	uint32_t graphicsQueueFamilyIndex = -1;
	uint32_t presentQueueFamilyIndex = -1;
	uint32_t transferQueueFamilyIndex = -1;
	uint32_t computeQueueFamilyIndex = -1;
};

struct VulkanPhysicalDeviceRequirements {
	bool graphics;
	bool present;
	bool compute;
	bool transfer;
	list<const char*> deviceExtensionNames;
	bool samplerAnisotropy;
	bool discreteGpu;
};

struct VulkanSwapchainSupportInfo {
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	list<VkSurfaceFormatKHR> formats;
	list<VkPresentModeKHR> presentModes;
};

class VulkanDevice {
public:
	VulkanDevice(const VulkanBackend* backend);
	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;
	~VulkanDevice();

	AINLINE operator VkDevice() const { return m_LogicalDevice; }
	AINLINE operator VkPhysicalDevice() const { return m_PhysicalDevice; }

	AINLINE VkDevice GetDevice() const { return m_LogicalDevice; }
	AINLINE VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
	AINLINE const VulkanSwapchainSupportInfo& GetSwapchainSupport() const { return m_SwapchainSupport; }
	AINLINE VulkanQueueFamilyIndices GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
	VkFormat DetectDepthFormat(bool useStencil) const;

private:
	void ChoosePhysicalDevice();
	void CreateLogicalDevice();
	VulkanSwapchainSupportInfo QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;
	bool PhysicalDeviceMeetsRequirements(
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface,
		const VkPhysicalDeviceProperties& properties,
		const VkPhysicalDeviceFeatures& features,
		const VulkanPhysicalDeviceRequirements& requirements,
		VulkanQueueFamilyIndices& outQueueFamilyIndices,
		VulkanSwapchainSupportInfo& outSwapchainSupportInfo);

private:
	const VulkanBackend* backend = nullptr;
	VkDevice m_LogicalDevice = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties m_DeviceProperties{};
	VkPhysicalDeviceMemoryProperties m_DeviceMemoryProperties{};
	VkPhysicalDeviceFeatures m_DeviceFeatures{};
	VulkanQueueFamilyIndices m_QueueFamilyIndices{};
	VulkanSwapchainSupportInfo m_SwapchainSupport{};
	VkQueue m_PresentQueue = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	VkQueue m_TransferQueue = VK_NULL_HANDLE;
};