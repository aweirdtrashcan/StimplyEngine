#pragma once

#include "defines.h"
#include "containers/list.h"

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(call) { 																							\
	VkResult res = (call);																							\
	if (res != VK_SUCCESS) { 																						\
		throw RendererException("%s failed with %s in %s:%d", #call, string_VkResult(res), __FILE__, __LINE__);		\
	} 																												\
}

struct VulkanSwapchainExtent {
	operator VkExtent2D() {
		return { width, height };
	}
	uint32_t width;
	uint32_t height;
};

struct VulkanRenderPassRect {
	union {
		struct {
			float x;
			float y;
			float width;
			float height;
		};

		struct {
			float r;
			float g;
			float b;
			float a;
		};
	};
};
