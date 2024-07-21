#include "vulkan_internals.h"

bool create_framebuffer(const internal_vulkan_renderer_state* state, VkRenderPass renderpass, uint32_t attachment_count, VkImageView* attachments, uint32_t width, uint32_t height, VkFramebuffer* out_framebuffer) {
    VkFramebufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.renderPass = renderpass;
    create_info.attachmentCount = attachment_count;
    create_info.pAttachments = attachments;
    create_info.width = width;
    create_info.height = height;
    create_info.layers = 1;
    
    vk_result(vkCreateFramebuffer(state->logical_device, &create_info, state->allocator, out_framebuffer));

    return true;
}
