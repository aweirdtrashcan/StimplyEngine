#pragma once

#include <defines.h>
#include <cstdint>
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
    uint32_t memory_property_flags;
    void* memory_pointer;
};

struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule shader_module;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
};

static inline constexpr int32_t OBJECT_SHADER_STAGE_COUNT = 2;

struct vulkan_pipeline {
    VkPipeline pipeline;
    VkPipelineLayout layout;
};

struct vulkan_shader {
    vulkan_shader_stage stages[OBJECT_SHADER_STAGE_COUNT];
    vulkan_pipeline pipeline;
};

struct vulkan_pipeline_create_info {
    HANDLE state;
    VkRenderPass renderpass;
    uint32_t attribute_count;
    VkVertexInputAttributeDescription* attributes;
    uint32_t descriptor_set_layout_count;
    VkDescriptorSetLayout* descriptor_set_layouts;
    uint32_t stage_count;
    VkPipelineShaderStageCreateInfo* stages;
    VkViewport viewport;
    VkRect2D scissor;
    bool is_wireframe;
};