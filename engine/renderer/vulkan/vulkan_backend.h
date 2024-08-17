#pragma once

#include "renderer/renderer_backend.h"
#include "vulkan_types.inl"

#include "containers/list.h"

class VulkanDevice;
class VulkanSwapchain;

class VulkanBackend : public RendererBackend {
public:
	VulkanBackend(const char* applicationName, const Window& window);
	virtual ~VulkanBackend() override;

	virtual void Resized(uint32_t width, uint32_t height) override;
    virtual bool BeginFrame(float deltaTime) override;
    virtual bool EndFrame(float deltaTime) override;
	virtual void WaitDeviceIdle() override;

	AINLINE VkInstance GetVulkanInstance() const { return m_Instance; }
	AINLINE VkSurfaceKHR GetVulkanSurface() const { return m_Surface; }
	AINLINE VkAllocationCallbacks* GetVulkanAllocator() const { return m_Allocator; }
	AINLINE const VulkanDevice& GetVulkanDevice() const { return *m_Device; }
	uint32_t FindMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperties) const;

private:
	static VkBool32 PrintDebugLayer(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	static VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

	list<const char*> GetRequiredInstanceLayers();

private:
	VkAllocationCallbacks* m_Allocator = nullptr;
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_Messenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VulkanDevice* m_Device = nullptr;
	VulkanSwapchain* m_Swapchain = nullptr;
};