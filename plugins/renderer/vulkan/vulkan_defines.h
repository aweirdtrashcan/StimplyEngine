#pragma once

#include <defines.h>
#include <cstdint>
#include <vulkan/vulkan_core.h>

#include <containers/list.h>

struct vulkan_image {
    const char* name;
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t mip_levels;
    VkImageAspectFlags image_aspect;
    VkImageLayout image_layout;
};

struct vulkan_texture {
    vulkan_image image;
    VkSampler sampler;
};

struct gpu_buffer {
    uint64_t size;
    VkBuffer buffer;
    VkDeviceMemory memory;
    uint32_t memory_property_flags;
    void* memory_pointer;
};

struct vulkan_shader_stage {
    VkShaderModuleCreateInfo shader_create_info;
    VkShaderModule shader_module;
    VkPipelineShaderStageCreateInfo pipeline_stage_create_info;
};

static inline constexpr int32_t MAX_SHADER_COUNT = 10;

struct vulkan_pipeline {
    VkPipeline pipeline;
    VkPipelineLayout layout;
};

struct vulkan_descriptor_pool {
    VkDescriptorPool pool;
    VkDescriptorType type_bits;
    VkDescriptorPoolCreateFlags flags;
};  

struct vulkan_descriptor_set {
    VkDescriptorSet set;
    VkDescriptorType type_bits;
    VkDescriptorSetLayout set_layout;
};

struct vulkan_shader_bundle {
    uint32_t shader_count;
    vulkan_shader_stage shaders[MAX_SHADER_COUNT];    
    vulkan_pipeline pipeline;
    vulkan_descriptor_pool shader_descriptor_pool;
    VkDescriptorSetLayout shader_set_layout;
    list<vulkan_descriptor_set> shader_descriptor_sets;
};

struct vulkan_pipeline_create_info {
    HANDLE state;
    VkRenderPass renderpass;
    uint32_t attribute_count;
    VkVertexInputAttributeDescription* attributes;
    uint32_t descriptor_set_layout_count;
    vulkan_descriptor_set* descriptor_set_layouts;
    uint32_t stage_count;
    vulkan_shader_stage* stages;
    VkViewport viewport;
    VkRect2D scissor;
    bool is_wireframe;
};

struct vulkan_shader_binding_create_info {
    uint32_t binding;
    uint32_t descriptor_count;
    VkDescriptorType descriptor_type;
    /* at which stage this binding is available */
    VkShaderStageFlagBits stage;
};

struct vulkan_shader_bundle_create_info {
    uint32_t shader_count;
    /* stages per shader_count*/
    /* this tell if a shader is a vertex, fragment, etc */
    VkShaderStageFlagBits* stages;
    /* if null, will be "main" */
    const char* entry_point;
    /* paths per shader_count */
    const char** paths;
    uint32_t binding_count;
    vulkan_shader_binding_create_info* bindings;
};

struct vulkan_internal_render_data {
    gpu_buffer* model_gpu_buffer;
    vulkan_texture* texture;
};