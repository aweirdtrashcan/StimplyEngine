#include "core/logger.h"
#include "platform/platform.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

#include <DirectXMath.h>

bool create_vulkan_shader(internal_vulkan_renderer_state* state, vulkan_shader* out_shader) {
    VkShaderStageFlagBits stage_types[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    const char* names[] = {
        "./mvp_vert.spv",
        "./mvp_frag.spv"
    };

    for (uint32_t i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        if (!create_shader_module(state, names[i], stage_types[i], i, out_shader->stages)) {
            Logger::debug("Failed to create shader module for: %s", names[i]);
            return false;
        }  
    }

    VkViewport viewport;
    VkRect2D scissor;

    get_viewport_and_scissor(state->surface_capabilities, &viewport, &scissor);

    uint32_t offset = 0;
    static constexpr uint32_t attribute_count = 1;
    VkVertexInputAttributeDescription attributes[attribute_count];
    uint32_t sizes[attribute_count] = {
        sizeof(DirectX::XMFLOAT3)
    };
    VkFormat formats[attribute_count] {
        VK_FORMAT_R32G32B32_SFLOAT
    };

    for (uint32_t i = 0; i < attribute_count; i++) {
        attributes[i].binding = 0;
        attributes[i].location = i;
        attributes[i].format = formats[i];
        attributes[i].offset = offset;
        offset += sizes[i];
    }

    VkPipelineShaderStageCreateInfo stages[2];

    for (uint32_t i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        stages[i] = out_shader->stages[i].shader_stage_create_info;
    }

    vulkan_pipeline_create_info create_info;
    create_info.state = state;
    create_info.renderpass = state->main_renderpass;
    create_info.attribute_count = (uint32_t)std::size(attributes);
    create_info.attributes = attributes;
    create_info.descriptor_set_layout_count = 0;
    create_info.descriptor_set_layouts = nullptr;
    create_info.stage_count = OBJECT_SHADER_STAGE_COUNT;
    create_info.stages = stages;
    create_info.viewport = viewport;
    create_info.scissor = scissor;
    create_info.is_wireframe = false;

    if (!create_pipeline(&create_info, &out_shader->pipeline)) {
        Logger::fatal("Failed to create graphics pipeline");
                return false;
    }

    return true;
}

bool destroy_vulkan_shader(internal_vulkan_renderer_state* state, vulkan_shader* shader) {
    for (uint32_t i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        destroy_shader_module(state, &shader->stages[i]);
    }

    destroy_pipeline(state, &shader->pipeline);

    Platform::zero_memory(shader, sizeof(*shader));

    return true;
}

bool vulkan_shader_use(internal_vulkan_renderer_state* state, vulkan_shader* shader) {
    return true;
}

bool create_shader_module(const internal_vulkan_renderer_state* state, const char* shader_path, VkShaderStageFlagBits shader_stage_flag, uint32_t stage_index, vulkan_shader_stage* shader_stage) {
    binary_info shader = Platform::read_binary(shader_path);
    
    if (shader.size <= 0) {
        return false;
    }

    shader_stage[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_stage[stage_index].create_info.pNext = nullptr;
    shader_stage[stage_index].create_info.flags = 0;
    shader_stage[stage_index].create_info.codeSize = shader.size;
    shader_stage[stage_index].create_info.pCode = (uint32_t*)shader.binary;
    
    vk_result(vkCreateShaderModule(
        state->logical_device, 
        &shader_stage[stage_index].create_info, 
        state->allocator, 
        &shader_stage[stage_index].shader_module));

    shader_stage[stage_index].shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage[stage_index].shader_stage_create_info.pNext = nullptr;
    shader_stage[stage_index].shader_stage_create_info.flags = 0;
    shader_stage[stage_index].shader_stage_create_info.stage = shader_stage_flag;
    shader_stage[stage_index].shader_stage_create_info.module = shader_stage[stage_index].shader_module;
    shader_stage[stage_index].shader_stage_create_info.pName = "main";
    shader_stage[stage_index].shader_stage_create_info.pSpecializationInfo = nullptr;

    return true;
}

bool destroy_shader_module(const internal_vulkan_renderer_state* state, vulkan_shader_stage* shader_stage) {
    vkDestroyShaderModule(state->logical_device, shader_stage->shader_module, state->allocator);
    Platform::zero_memory(shader_stage, sizeof(*shader_stage));
    return true;
}
