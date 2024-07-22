#include "core/logger.h"
#include "platform/platform.h"
#include "vulkan_defines.h"
#include "vulkan_internals.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

bool internal_create_graphics_pipeline(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities,
    const char** vertex_shader_paths, uint32_t vertex_shader_count, const char** fragment_shader_paths, uint32_t fragment_shader_count,
    const VkPipelineVertexInputStateCreateInfo* vertex_input, VkPipelineLayout pipeline_layout, VkPipeline* out_pipeline);

bool destroy_graphics_pipeline_layout(internal_vulkan_renderer_state* state, VkPipelineLayout pipeline_layout) {
    vkDestroyPipelineLayout(state->logical_device, pipeline_layout, state->allocator);

    return true;
}

bool destroy_pipeline(internal_vulkan_renderer_state* state, VkPipeline pipeline) {
    vkDestroyPipeline(state->logical_device, pipeline, state->allocator);

    return true;
}

bool create_mvp_pipeline_layout(internal_vulkan_renderer_state* state) {
    if (!state->graphics_set_layouts[LAYOUT_MVP]) {
        Logger::fatal("MVP Descriptor Set Layout is nullptr");
        return false;
    }

    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.setLayoutCount = 1;
    create_info.pSetLayouts = &state->graphics_set_layouts[LAYOUT_MVP];
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;

    vk_result(vkCreatePipelineLayout(
        state->logical_device,
        &create_info,
        state->allocator,
        &state->graphics_pipeline_layouts[LAYOUT_MVP]));

    return true;
}

bool create_mvp_pipeline(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    if (state->graphics_pipelines[LAYOUT_MVP]) {
        Logger::fatal("create_mvp_pipeline: pipeline state is already created");
        return false;
    }

    VkPipelineVertexInputStateCreateInfo vertex_input;
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.pNext = nullptr;
    vertex_input.flags = 0;
    vertex_input.vertexBindingDescriptionCount = 0;
    vertex_input.pVertexBindingDescriptions = nullptr;
    vertex_input.vertexAttributeDescriptionCount = 0;
    vertex_input.pVertexAttributeDescriptions = nullptr;

    list<const char*> vertex_shader_paths;
    list<const char*> fragment_shader_paths;

    vertex_shader_paths.push_back("./mvp_vert.spv");
    fragment_shader_paths.push_back("./mvp_frag.spv");

    return internal_create_graphics_pipeline(state, surface_capabilities, vertex_shader_paths.data(), 
        vertex_shader_paths.size_u32(), fragment_shader_paths.data(), 
        fragment_shader_paths.size_u32(), &vertex_input, state->graphics_pipeline_layouts[LAYOUT_MVP], &state->graphics_pipelines[LAYOUT_MVP]);
}

bool internal_create_graphics_pipeline(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities,
    const char** vertex_shader_paths, uint32_t vertex_shader_count, const char** fragment_shader_paths, uint32_t fragment_shader_count,
    const VkPipelineVertexInputStateCreateInfo* vertex_input, VkPipelineLayout pipeline_layout, VkPipeline* out_pipeline) {

    uint32_t shader_count = vertex_shader_count + fragment_shader_count;
    list<VkShaderModule> vertex_shaders(vertex_shader_count);
    list<VkShaderModule> fragment_shaders(fragment_shader_count);

    list<VkPipelineShaderStageCreateInfo> shaders(shader_count);

    for (uint32_t i = 0; i < vertex_shader_count; i++) {
        VkShaderModule vert;
        if (!create_shader_module(state, vertex_shader_paths[i], &vert)) {
            Logger::fatal("Failed to create shader module: %s", vertex_shader_paths[i]);
            return false;
        }

        vertex_shaders.push_back(vert);

        VkPipelineShaderStageCreateInfo vertex_create_info;
        vertex_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_create_info.pNext = nullptr;
        vertex_create_info.flags = 0;
        vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_create_info.module = vertex_shaders[i];
        vertex_create_info.pName = "main";
        vertex_create_info.pSpecializationInfo = nullptr;

        shaders.push_back(vertex_create_info);
    }

    for (uint32_t i = 0; i < fragment_shader_count; i++) {
        VkShaderModule frag;
        if (!create_shader_module(state, fragment_shader_paths[i], &frag)) {
            Logger::fatal("Failed to create shader module: %s", fragment_shader_paths[i]);
            return false;
        }

        fragment_shaders.push_back(frag);

        VkPipelineShaderStageCreateInfo fragment_create_info;
        fragment_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_create_info.pNext = nullptr;
        fragment_create_info.flags = 0;
        fragment_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;  
        fragment_create_info.module = fragment_shaders[i];
        fragment_create_info.pName = "main";
        fragment_create_info.pSpecializationInfo = nullptr;

        shaders.push_back(fragment_create_info);
    }

    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.pNext = nullptr;
    input_assembly.flags = 0;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineTessellationStateCreateInfo tesselation;
    tesselation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tesselation.pNext = nullptr;
    tesselation.flags = 0;
    tesselation.patchControlPoints = 0;

    VkViewport vp;
    VkRect2D scissor;
    get_viewport_and_scissor(surface_capabilities, &vp, &scissor);

    VkPipelineViewportStateCreateInfo viewport;
    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.pNext = nullptr;
    viewport.flags = 0;
    viewport.viewportCount = 1;
    viewport.pViewports = &vp;
    viewport.scissorCount = 1;
    viewport.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterization;
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.pNext = nullptr;
    rasterization.flags = 0;
    rasterization.depthClampEnable = VK_TRUE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization.depthBiasEnable = VK_FALSE;
    rasterization.depthBiasConstantFactor = 0.0f;
    rasterization.depthBiasClamp = 0.0f;
    rasterization.depthBiasSlopeFactor = 0.0f;
    rasterization.lineWidth = 1.0f;

    VkSampleMask sample_mask = UINT32_MAX;

    VkPipelineMultisampleStateCreateInfo multisample;
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.pNext = nullptr;
    multisample.flags = 0;
    multisample.rasterizationSamples = state->samples;
    multisample.sampleShadingEnable = state->samples > 1 ? VK_TRUE : VK_FALSE;
    multisample.minSampleShading = 10.0f;
    multisample.pSampleMask = &sample_mask;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil;
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.pNext = nullptr;
    depth_stencil.flags = 0;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;

    VkPipelineColorBlendAttachmentState color_attachment;
    color_attachment.blendEnable = VK_FALSE;
    color_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_attachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend;
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.pNext = nullptr;
    color_blend.flags = 0;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_CLEAR;
    color_blend.attachmentCount = 1;
    color_blend.pAttachments = &color_attachment;
    memset(&color_blend.blendConstants, 0,  4 * sizeof(float));

    VkDynamicState states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state;
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pNext = nullptr;
    dynamic_state.flags = 0;
    dynamic_state.dynamicStateCount = std::size(states);
    dynamic_state.pDynamicStates = states;

    VkGraphicsPipelineCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.stageCount = shader_count;
    create_info.pStages = shaders.data();
    create_info.pVertexInputState = vertex_input;
    create_info.pInputAssemblyState = &input_assembly;
    create_info.pTessellationState = &tesselation;
    create_info.pViewportState = &viewport;
    create_info.pRasterizationState = &rasterization;
    create_info.pMultisampleState = &multisample;
    create_info.pDepthStencilState = &depth_stencil;
    create_info.pColorBlendState = &color_blend;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = pipeline_layout;
    create_info.renderPass = state->main_renderpass;
    create_info.subpass = 0;
    create_info.basePipelineHandle = nullptr;
    create_info.basePipelineIndex = 0;

    VkResult result = vkCreateGraphicsPipelines(
        state->logical_device, 
        nullptr, 
        1, 
        &create_info, 
        state->allocator, 
        out_pipeline);

    for (VkShaderModule module : vertex_shaders) {
        vkDestroyShaderModule(state->logical_device, module, state->allocator);
    }

    for (VkShaderModule module : fragment_shaders) {
        vkDestroyShaderModule(state->logical_device, module, state->allocator);
    }

    if (result != VK_SUCCESS) {
        return false;
    }

    return true;
}