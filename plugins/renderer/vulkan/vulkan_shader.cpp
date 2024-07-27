#include "core/logger.h"
#include "platform/platform.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <renderer/renderer_exception.h>

#include <DirectXMath.h>

bool create_vulkan_shader_bundle(const internal_vulkan_renderer_state* state, const vulkan_shader_bundle_create_info* create_info, vulkan_shader_bundle* out_shader) {
    uint32_t shader_count = create_info->shader_count;

    if (out_shader == nullptr) {
        Logger::warning("create_vulkan_shader: out_shader is nullptr");
        return false;
    }

    if (shader_count > MAX_SHADER_COUNT) {
        Logger::warning("create_vulkan_shader_bundle: Can't create more shaders than %d", MAX_SHADER_COUNT);
        return false;
    }

    Platform::ZeroMemory(out_shader, sizeof(*out_shader));

    list<VkDescriptorType> descriptor_types;
    list<VkDescriptorSetLayoutBinding> bindings;

    // Global binding
    VkDescriptorSetLayoutBinding global_binding;
    global_binding.binding = 0;
    global_binding.descriptorCount = 1;
    global_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_binding.pImmutableSamplers = nullptr;
    global_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptor_types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    bindings.push_back(global_binding);

    for (uint32_t i = 0; i < shader_count; i++) {
        if (!create_shader_module(state, create_info->paths[i], create_info->stages[i], &out_shader->shaders[i])) {
            Logger::warning("create_vulkan_shader: Failed to create shader module for path: %s", create_info->paths[i]);
            destroy_vulkan_shader_bundle(state, out_shader);
            return false;
        }
    }

    // bindings
    for (uint32_t i = 0; i < create_info->binding_count; i++) {
        if (create_info->bindings[i].binding == 0) {
            Logger::warning("create_vulkan_shader: vulkan_shader_bundle_create_info::bindings[%d]->binding shouldn't be 0. 0 is the binding for the global buffer");
            destroy_vulkan_shader_bundle(state, out_shader);
            return false;
        }
        VkDescriptorSetLayoutBinding set_binding;
        set_binding.binding = create_info->bindings[i].binding;
        set_binding.descriptorCount = create_info->bindings[i].binding;
        set_binding.descriptorType = create_info->bindings[i].descriptor_type;
        set_binding.pImmutableSamplers = nullptr;
        set_binding.stageFlags = create_info->bindings[i].stage;
        bindings.push_back(set_binding);

        descriptor_types.push_back(set_binding.descriptorType);
    }

    if (!create_descriptor_pool(state, 
        descriptor_types.data(),
        descriptor_types.size_u32(),
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 
        &out_shader->shader_descriptor_pool, 
        state->num_frames)) {
        Logger::warning("create_vulkan_shader_bundle: Failed to create global descriptor pool");
        destroy_vulkan_shader_bundle(state, out_shader);
        return false;
    }    

    if (!create_descriptor_set_layout(state, bindings.size_u32(), bindings.data(), &out_shader->shader_set_layout)) {
        Logger::warning("create_vulkan_shader: Failed to create descriptor set layout");
        destroy_vulkan_shader_bundle(state, out_shader);
        return false;
    }

    out_shader->shader_descriptor_sets.resize(bindings.size());

    for (uint32_t i = 0; i < bindings.size_u32(); i++) {
        if (!allocate_descriptor_set(state, &out_shader->shader_descriptor_pool, out_shader->shader_set_layout, &out_shader->shader_descriptor_sets[i])) {
            Logger::warning("create_vulkan_shader: Failed to allocate shader descriptor set");
            destroy_vulkan_shader_bundle(state, out_shader);
            return false;
        }
    }

    list<VkVertexInputAttributeDescription> vertex_input;
    create_default_vertex_input_attributes_layout(vertex_input);

    vulkan_pipeline_create_info pipeline_create_info;
    pipeline_create_info.state = (HANDLE)state;
    pipeline_create_info.renderpass = state->main_renderpass;
    pipeline_create_info.attribute_count = vertex_input.size_u32();
    pipeline_create_info.attributes = vertex_input.data();
    pipeline_create_info.descriptor_set_layout_count = out_shader->shader_descriptor_sets.size_u32();
    pipeline_create_info.descriptor_set_layouts = out_shader->shader_descriptor_sets.data();
    pipeline_create_info.stage_count = shader_count;
    pipeline_create_info.stages = out_shader->shaders;
    get_viewport_and_scissor(state->surface_capabilities, &pipeline_create_info.viewport, &pipeline_create_info.scissor);
    pipeline_create_info.is_wireframe = false;

    if (!create_pipeline(&pipeline_create_info, &out_shader->pipeline)) {
        Logger::warning("create_vulkan_shader_bundle: Failed to create shader's pipeline");
        destroy_vulkan_shader_bundle(state, out_shader);
        return false;
    }

    out_shader->shader_count = shader_count;

    return true;
}

bool destroy_vulkan_shader_bundle(const internal_vulkan_renderer_state* state, vulkan_shader_bundle* shader) {
    for (uint32_t i = 0; i < shader->shader_count; i++) {
        destroy_shader_module(state, &shader->shaders[i]);
    }

    destroy_descriptor_pool(state, &shader->shader_descriptor_pool);
    destroy_descriptor_set_layout(state, shader->shader_set_layout);
    destroy_pipeline(state, &shader->pipeline);

    Platform::ZeroMemory(shader, sizeof(*shader));

    return true;
}

bool update_vulkan_shader(const internal_vulkan_renderer_state* state, vulkan_shader_bundle* shader, const render_item* render_item) {
    throw RendererException("update_vulkan_shader: Unimplemented function");

    return true;
}

bool vulkan_shader_use(internal_vulkan_renderer_state* state, VkCommandBuffer command_buffer, vulkan_shader_bundle* shader) {
    pipeline_bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
    bind_descriptor_set(state, command_buffer, &shader->shader_descriptor_sets[state->current_frame_index], &shader->pipeline);
    return true;
}

bool create_shader_module(const internal_vulkan_renderer_state* state, const char* shader_path, VkShaderStageFlagBits shader_stage_flag, vulkan_shader_stage* shader_stage) {
    binary_info shader = Platform::OpenBinary(shader_path);
    
    if (shader.size <= 0) {
        return false;
    }

    shader_stage->shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_stage->shader_create_info.pNext = nullptr;
    shader_stage->shader_create_info.flags = 0;
    shader_stage->shader_create_info.codeSize = shader.size;
    shader_stage->shader_create_info.pCode = (uint32_t*)shader.binary;
    
    vk_result(vkCreateShaderModule(
        state->logical_device, 
        &shader_stage->shader_create_info, 
        state->allocator, 
        &shader_stage->shader_module));

    shader_stage->pipeline_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage->pipeline_stage_create_info.pNext = nullptr;
    shader_stage->pipeline_stage_create_info.flags = 0;
    shader_stage->pipeline_stage_create_info.stage = shader_stage_flag;
    shader_stage->pipeline_stage_create_info.module = shader_stage->shader_module;
    shader_stage->pipeline_stage_create_info.pName = "main";
    shader_stage->pipeline_stage_create_info.pSpecializationInfo = nullptr;

    Platform::CloseBinary(&shader);

    return true;
}

bool destroy_shader_module(const internal_vulkan_renderer_state* state, vulkan_shader_stage* shader_stage) {
    vkDestroyShaderModule(state->logical_device, shader_stage->shader_module, state->allocator);
    Platform::ZeroMemory(shader_stage, sizeof(*shader_stage));
    return true;
}
