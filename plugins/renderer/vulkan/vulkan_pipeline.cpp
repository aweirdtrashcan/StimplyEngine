#include "vulkan_internals.h"

bool create_naked_graphics_pipeline_layout(internal_vulkan_renderer_state* state) {
    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.setLayoutCount = 0;
    create_info.pSetLayouts = nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    
    vk_result(vkCreatePipelineLayout(
        state->logical_device, 
        &create_info, 
        state->allocator, 
        &state->graphics_pipeline_layouts[LAYOUT_NAKED]));

    return true;
}

bool create_naked_graphics_pipeline_state(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    if (state->graphics_pipelines[LAYOUT_NAKED]) {
        Logger::fatal("create_naked_graphics_pipeline_state: pipeline state is already created");
        return false;
    }

    VkShaderModule vertex_module;
    VkShaderModule fragment_module;

    create_shader_module(state, "./naked_vert.spv", &vertex_module);
    create_shader_module(state, "./naked_frag.spv", &fragment_module);

    VkPipelineShaderStageCreateInfo shaders[2];
    shaders[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaders[0].pNext = nullptr;
    shaders[0].flags = 0;
    shaders[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaders[0].module = vertex_module;
    shaders[0].pName = "main";
    shaders[0].pSpecializationInfo = nullptr;

    shaders[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaders[1].pNext = nullptr;
    shaders[1].flags = 0;
    shaders[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaders[1].module = fragment_module;
    shaders[1].pName = "main";
    shaders[1].pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertex_input;
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.pNext = nullptr;
    vertex_input.flags = 0;
    vertex_input.vertexBindingDescriptionCount = 0;
    vertex_input.pVertexBindingDescriptions = nullptr;
    vertex_input.vertexAttributeDescriptionCount = 0;
    vertex_input.pVertexAttributeDescriptions = nullptr;

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
    create_info.stageCount = std::size(shaders);
    create_info.pStages = shaders;
    create_info.pVertexInputState = &vertex_input;
    create_info.pInputAssemblyState = &input_assembly;
    create_info.pTessellationState = &tesselation;
    create_info.pViewportState = &viewport;
    create_info.pRasterizationState = &rasterization;
    create_info.pMultisampleState = &multisample;
    create_info.pDepthStencilState = &depth_stencil;
    create_info.pColorBlendState = &color_blend;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = state->graphics_pipeline_layouts[LAYOUT_NAKED];
    create_info.renderPass = state->main_renderpass;
    create_info.subpass = 0;
    create_info.basePipelineHandle = nullptr;
    create_info.basePipelineIndex = 0;

    vk_result(vkCreateGraphicsPipelines(
        state->logical_device, 
        nullptr, 
        1, 
        &create_info, 
        state->allocator, 
        &state->graphics_pipelines[LAYOUT_NAKED]));

    destroy_shader_module(state, vertex_module);
    destroy_shader_module(state, fragment_module);

    return true;
}

bool destroy_naked_graphics_pipeline_layout(internal_vulkan_renderer_state* state) {
    vkDestroyPipelineLayout(state->logical_device, state->graphics_pipeline_layouts[LAYOUT_NAKED], state->allocator);

    return true;
}

bool destroy_naked_graphics_pipeline_state(internal_vulkan_renderer_state* state) {
    vkDestroyPipeline(state->logical_device, state->graphics_pipelines[LAYOUT_NAKED], state->allocator);

    return true;
}

bool create_mvp_pipeline_layout(internal_vulkan_renderer_state* state) {
    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.setLayoutCount = 0;
    create_info.pSetLayouts = nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;

    vk_result(vkCreatePipelineLayout(
        state->logical_device,
        &create_info,
        state->allocator,
        &state->graphics_pipeline_layouts[LAYOUT_NAKED]));

    return true;
}

bool create_mvp_pipeline(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    return true;
}