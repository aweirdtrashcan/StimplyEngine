#include "vulkan_internals.h"

#include <core/logger.h>
#include "vulkan_defines.h"

#include <iterator>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <cstring>
#include <DirectXMath.h>

bool create_pipeline_layout(const internal_vulkan_renderer_state* state, VkDescriptorSetLayout* set_layouts, uint32_t set_layout_count, VkPipelineLayout* out_pipeline_layout) {
    VkPushConstantRange push_constant_range;
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(DirectX::XMFLOAT4X4);

    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = 1;
    create_info.pPushConstantRanges = &push_constant_range;

    vk_result(vkCreatePipelineLayout(
        state->logical_device,
        &create_info,
        state->allocator,
        out_pipeline_layout));

    return true;
}

bool create_pipeline(vulkan_pipeline_create_info* create_info, vulkan_pipeline* out_pipeline) {
    if (!create_info || !out_pipeline) {
        Logger::warning("create_pipeline: either create_info or out_pipeline are nullptr");
        return false;
    }

    internal_vulkan_renderer_state* state = (internal_vulkan_renderer_state*)create_info->state;

    VkVertexInputBindingDescription vertex_input_bindings[1];
    vertex_input_bindings[0].binding = 0;
    vertex_input_bindings[0].stride = sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT2);
    vertex_input_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertex_input;
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.pNext = nullptr;
    vertex_input.flags = 0;
    vertex_input.vertexBindingDescriptionCount = (uint32_t)std::size(vertex_input_bindings);
    vertex_input.pVertexBindingDescriptions = vertex_input_bindings;
    vertex_input.vertexAttributeDescriptionCount = create_info->attribute_count;
    vertex_input.pVertexAttributeDescriptions = create_info->attributes;

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

    VkPipelineViewportStateCreateInfo viewport;
    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.pNext = nullptr;
    viewport.flags = 0;
    viewport.viewportCount = 1;
    viewport.pViewports = &create_info->viewport;
    viewport.scissorCount = 1;
    viewport.pScissors = &create_info->scissor;

    VkPipelineRasterizationStateCreateInfo rasterization;
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.pNext = nullptr;
    rasterization.flags = 0;
    rasterization.depthClampEnable = VK_TRUE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = create_info->is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
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
    color_attachment.blendEnable = VK_TRUE;
    color_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend;
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.pNext = nullptr;
    color_blend.flags = 0;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_COPY;
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
    dynamic_state.dynamicStateCount = (uint32_t)std::size(states);
    dynamic_state.pDynamicStates = states;

    VkDescriptorSetLayout set_layouts[10];
    for (uint32_t i = 0; i < create_info->descriptor_set_layout_count; i++) {
        set_layouts[i] = create_info->descriptor_set_layouts[i].set_layout;
    }

    if (!create_pipeline_layout(state, set_layouts, create_info->descriptor_set_layout_count, &out_pipeline->layout)) {
        return false;
    }

    VkPipelineShaderStageCreateInfo stages[10];
    for (uint32_t i = 0; i < create_info->stage_count; i++) {
        stages[i] = create_info->stages[i].pipeline_stage_create_info;
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info;
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = nullptr;
    pipeline_create_info.flags = 0;
    pipeline_create_info.stageCount = create_info->stage_count;
    pipeline_create_info.pStages = stages;
    pipeline_create_info.pVertexInputState = &vertex_input;
    pipeline_create_info.pInputAssemblyState = &input_assembly;
    pipeline_create_info.pTessellationState = &tesselation;
    pipeline_create_info.pViewportState = &viewport;
    pipeline_create_info.pRasterizationState = &rasterization;
    pipeline_create_info.pMultisampleState = &multisample;
    pipeline_create_info.pDepthStencilState = &depth_stencil;
    pipeline_create_info.pColorBlendState = &color_blend;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.layout = out_pipeline->layout;
    pipeline_create_info.renderPass = state->main_renderpass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = 0;

    VkResult result = vkCreateGraphicsPipelines(
        state->logical_device, 
        nullptr, 
        1, 
        &pipeline_create_info, 
        state->allocator, 
        &out_pipeline->pipeline);

    if (result != VK_SUCCESS) {
        if (out_pipeline->layout) {
            destroy_pipeline_layout(state, out_pipeline->layout);
            out_pipeline->layout = nullptr;
        }
        return false;
    }

    return true;
}

bool destroy_pipeline_layout(const internal_vulkan_renderer_state* state, VkPipelineLayout pipeline_layout) {
    vkDestroyPipelineLayout(state->logical_device, pipeline_layout, state->allocator);

    return true;
}

bool destroy_pipeline(const internal_vulkan_renderer_state* state, vulkan_pipeline* pipeline) {
    vkDestroyPipeline(state->logical_device, pipeline->pipeline, state->allocator);
    destroy_pipeline_layout(state, pipeline->layout);

    pipeline->pipeline = nullptr;
    pipeline->layout = nullptr;

    return true;
}

bool pipeline_bind(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point, vulkan_pipeline* pipeline) {
    vkCmdBindPipeline(command_buffer, bind_point, pipeline->pipeline);
    return true;
}

bool create_default_vertex_input_attributes_layout(list<VkVertexInputAttributeDescription>& states) {
    VkVertexInputAttributeDescription desc;
    // pos
    desc.binding = 0;
    desc.location = 0;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = 0;
    states.push_back(desc);
    // texcoord
    desc.binding = 0;
    desc.location = 1;
    desc.format = VK_FORMAT_R32G32_SFLOAT;
    desc.offset = 12;
    states.push_back(desc);

    return true;
}