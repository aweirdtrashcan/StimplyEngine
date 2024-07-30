#include "vulkan_internals.h"

#include <iterator>

bool create_render_pass(internal_vulkan_renderer_state* state) {
    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = state->back_buffer_format;
    color_attachment.samples = state->samples;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment;
    depth_attachment.flags = 0;
    depth_attachment.format = state->depth_buffer.format;
    depth_attachment.samples = state->samples;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription attachments[] = {
        color_attachment,
        depth_attachment
    };

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = depth_attachment.finalLayout;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkSubpassDependency dependencies;
    dependencies.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies.dstSubpass = 0;
    dependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.srcAccessMask = 0;
    dependencies.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies.dependencyFlags = 0;

    VkRenderPassCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.attachmentCount = std::size(attachments);
    create_info.pAttachments = attachments;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependencies;

    vk_result(vkCreateRenderPass(state->logical_device, &create_info, state->allocator, &state->main_renderpass));

    return true;
}

bool destroy_render_pass(internal_vulkan_renderer_state* state) {
    vkDestroyRenderPass(state->logical_device, state->main_renderpass, state->allocator);
    state->main_renderpass = nullptr;
    return true;
}

bool begin_render_pass(VkRenderPass render_pass, VkCommandBuffer command_buffer, VkFramebuffer framebuffer, VkRect2D render_area, uint32_t clear_value_count, const VkClearValue* clear_values) {
    VkRenderPassBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = framebuffer;
    begin_info.renderArea = render_area;
    begin_info.clearValueCount = clear_value_count;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

bool end_render_pass(VkCommandBuffer command_buffer) {
    vkCmdEndRenderPass(command_buffer);
    return true;
}