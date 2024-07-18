#pragma once

#include <vulkan/vulkan_core.h>

struct vulkan_image {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkFormat format;
    uint32_t width;
    uint32_t height;
};