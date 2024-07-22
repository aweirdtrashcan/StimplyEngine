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

struct gpu_buffer {
    uint64_t size;
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* memory_pointer;
};

enum vulkan_layouts {
    LAYOUT_MVP,

    LAYOUT_MAX
};