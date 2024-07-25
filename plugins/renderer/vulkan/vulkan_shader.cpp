#include "core/logger.h"
#include "platform/platform.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"
#include <renderer/global_uniform_object.h>

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
            Logger::warning("Failed to create shader module for: %s", names[i]);
            return false;
        }  
    }

    if (!create_descriptor_pool(state, 
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 
        &out_shader->global_descriptor_pool, 
        state->num_frames)) {
        Logger::warning("Failed to create global descriptor pool");
        return false;
    }

    VkDescriptorSetLayoutBinding bindings[1];
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    if (!create_descriptor_set_layout(state, (uint32_t)std::size(bindings), bindings, &out_shader->global_descriptor_set_layout)) {
        Logger::warning("Failed to create global descriptor set layout");
        return false;
    }

    for (uint32_t i = 0; i < state->num_frames; i++) {
        out_shader->global_descriptor_sets.resize(state->num_frames);
        if (!allocate_descriptor_set(state, &out_shader->global_descriptor_pool, out_shader->global_descriptor_set_layout, &out_shader->global_descriptor_sets[i])) {
            Logger::warning("Failed to create global descriptor sets");
            return false;
        }
        uint64_t offset = i * sizeof(GlobalUniformObject);
        update_descriptor_set(state, state->global_uniform_buffer.buffer, offset, sizeof(GlobalUniformObject), &out_shader->global_descriptor_sets[i], 0);
    }

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
    create_info.descriptor_set_layout_count = 1;
    create_info.descriptor_set_layouts = &out_shader->global_descriptor_set_layout;
    create_info.stage_count = OBJECT_SHADER_STAGE_COUNT;
    create_info.stages = stages;
    get_viewport_and_scissor(state->surface_capabilities, &create_info.viewport, &create_info.scissor);
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

    destroy_descriptor_pool(state, &shader->global_descriptor_pool);
    destroy_descriptor_set_layout(state, shader->global_descriptor_set_layout);
    destroy_pipeline(state, &shader->pipeline);

    Platform::ZeroMemory(shader, sizeof(*shader));

    return true;
}

bool vulkan_shader_use(internal_vulkan_renderer_state* state, VkCommandBuffer command_buffer, vulkan_shader* shader) {
    pipeline_bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
    bind_descriptor_set(state, command_buffer, &shader->global_descriptor_sets[state->current_frame_index], &shader->pipeline);
    return true;
}

bool create_shader_module(const internal_vulkan_renderer_state* state, const char* shader_path, VkShaderStageFlagBits shader_stage_flag, uint32_t stage_index, vulkan_shader_stage* shader_stage) {
    binary_info shader = Platform::ReadBinary(shader_path);
    
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

    Platform::UFree(shader.binary);

    return true;
}

bool destroy_shader_module(const internal_vulkan_renderer_state* state, vulkan_shader_stage* shader_stage) {
    vkDestroyShaderModule(state->logical_device, shader_stage->shader_module, state->allocator);
    Platform::ZeroMemory(shader_stage, sizeof(*shader_stage));
    return true;
}
