#pragma once

#include "containers/list.h"
#include "vulkan_defines.h"
#include "../render_item.h"

#include <vulkan/vulkan.h>


#define vk_result(result)                       \
    {                                           \
        VkResult res = (result);                \
        if (res != VK_SUCCESS) return false;    \
    }                                           \


extern "C" {

struct internal_vulkan_renderer_state {
    VkInstance instance;
    void* window;
    VkAllocationCallbacks* allocator;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    uint32_t graphics_queue_index;
    VkQueue graphics_queue;
    VkDebugUtilsMessengerEXT messenger;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    list<VkImage> back_buffers;
    uint32_t mip_levels;
    VkSampleCountFlagBits samples;
    VkFormat back_buffer_format;
    list<VkImageView> back_buffer_views;
    list<VkFence> last_image_fence;
    uint32_t num_frames;
    VkCommandPool command_pool;
    list<VkCommandBuffer> graphics_command_buffers;
    list<VkFence> fences;
    list<VkSemaphore> image_acquired_semaphore;
    list<VkSemaphore> queue_complete_semaphore;
    vulkan_image depth_buffer;
    VkDescriptorPool uniform_descriptor_pool;
    uint32_t current_frame_index;
    uint32_t image_index;
    VkDescriptorSetLayout graphics_set_layouts[LAYOUT_MAX];
    VkPipeline graphics_pipelines[LAYOUT_MAX];
    VkPipelineLayout graphics_pipeline_layouts[LAYOUT_MAX];
    VkRenderPass main_renderpass;
    VkViewport viewport;
    VkRect2D scissor;
    list<VkFramebuffer> swapchain_framebuffers;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkClearValue clear_values[2];
    list<render_item> render_items;
};

VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData);

bool create_vulkan_instance(internal_vulkan_renderer_state* state, const char* name);
bool enumerate_required_instance_extensions(const internal_vulkan_renderer_state* state, list<const char*>& out_required_extensions);
bool enumerate_required_instance_layers(const internal_vulkan_renderer_state* state, list<const char*>& out_required_layers);

VkDebugUtilsMessengerCreateInfoEXT get_debug_utils_messenger_create_info();
bool enable_validation_layer(internal_vulkan_renderer_state* state);

bool select_physical_device(internal_vulkan_renderer_state* state);
bool create_logical_device(internal_vulkan_renderer_state* state, const VkPhysicalDeviceFeatures* features);
bool enumerate_required_device_extensions(const internal_vulkan_renderer_state* state, list<const char*>& out_required_extensions);
bool get_physical_device_queues(internal_vulkan_renderer_state* state);
bool query_optimal_back_buffer_format(const internal_vulkan_renderer_state* state, VkSurfaceFormatKHR* format);
bool destroy_device(const internal_vulkan_renderer_state* state);

bool create_surface(internal_vulkan_renderer_state* state);

bool create_swapchain(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities);
bool destroy_swapchain(internal_vulkan_renderer_state* state);
bool get_swapchain_back_buffers(internal_vulkan_renderer_state* state);
bool create_swapchain_back_buffer_views(internal_vulkan_renderer_state* state);
bool destroy_swapchain_image_views(internal_vulkan_renderer_state* state);

bool create_fence(const internal_vulkan_renderer_state* state, VkFence& out_fence, bool create_signaled);
bool destroy_fence(const internal_vulkan_renderer_state* state, VkFence fence);

bool create_semaphore(const internal_vulkan_renderer_state* state, VkSemaphore& out_semaphore);
bool create_swapchain_semaphores_and_fences(internal_vulkan_renderer_state* state);
bool destroy_swapchain_semaphores_and_fences(internal_vulkan_renderer_state* state);
/**
    Will create the depth VkImage, VkImageView, VkDeviceMemory, bind the memory, and assign the right format.
 */
bool create_depth_buffer(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities);
bool destroy_depth_buffer(internal_vulkan_renderer_state* state);

bool find_memory_type_index(const internal_vulkan_renderer_state* state, uint32_t supported_memory_type, VkMemoryPropertyFlags property_flags, uint32_t* out_memory_type_index);

bool create_descriptor_pool(const internal_vulkan_renderer_state* state, VkDescriptorType type, bool can_be_freed, VkDescriptorPool* out_descriptor_pool, uint32_t max_sets = 1000);
bool destroy_descriptor_pool(const internal_vulkan_renderer_state* state, VkDescriptorPool descriptor_pool);
bool create_descriptor_set(const internal_vulkan_renderer_state* state, VkDescriptorPool descriptor_pool, VkDescriptorSet* out_descriptor_set);
bool free_descriptor_set(const internal_vulkan_renderer_state* state, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
bool create_mvp_set_layout_binding();
bool create_descriptor_set_layout(const internal_vulkan_renderer_state* state, uint32_t binding_count, const VkDescriptorSetLayoutBinding* bindings, VkDescriptorSetLayout* out_set_layout);
bool destroy_descriptor_set_layout(const internal_vulkan_renderer_state* state, VkDescriptorSetLayout set_layout);

bool create_command_pool(const internal_vulkan_renderer_state* state, VkCommandPool* out_command_pool, uint32_t queueIndex);
bool destroy_command_pool(const internal_vulkan_renderer_state* state, VkCommandPool command_pool);
bool allocate_command_buffers(const internal_vulkan_renderer_state* state, VkCommandPool command_pool, uint32_t command_buffer_count, VkCommandBuffer* out_command_buffer);
bool free_command_buffers(const internal_vulkan_renderer_state* state, uint32_t command_buffer_count, VkCommandPool command_pool, VkCommandBuffer* command_buffers);

bool create_render_pass(internal_vulkan_renderer_state* state);
bool destroy_render_pass(internal_vulkan_renderer_state* state);

bool destroy_graphics_pipeline_layout(internal_vulkan_renderer_state* state, VkPipelineLayout pipeline_layout);
bool destroy_pipeline(internal_vulkan_renderer_state* state, VkPipeline pipeline);

bool get_viewport_and_scissor(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkViewport* out_viewport, VkRect2D* out_scissor);

bool create_shader_module(const internal_vulkan_renderer_state* state, const char* shader_path, VkShaderModule* out_shader_module);
bool destroy_shader_module(const internal_vulkan_renderer_state* state, VkShaderModule shader_module);

bool create_framebuffer(const internal_vulkan_renderer_state* state, VkRenderPass renderpass, uint32_t attachment_count, VkImageView* attachments, uint32_t width, uint32_t height, VkFramebuffer* out_framebuffer);
bool create_swapchain_framebuffers(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities);
bool destroy_swapchain_framebuffers(internal_vulkan_renderer_state* state);
bool recreate_swapchain(internal_vulkan_renderer_state* state);

bool begin_command_buffer(VkCommandBuffer command_buffer);
bool end_command_buffer(VkCommandBuffer command_buffer);

bool begin_render_pass(VkRenderPass render_pass, VkCommandBuffer command_buffer, VkFramebuffer framebuffer, VkRect2D render_area, uint32_t clear_value_count, const VkClearValue* clear_values);
bool end_render_pass(VkCommandBuffer command_buffer);

bool create_uploader_buffer(internal_vulkan_renderer_state* state, size_t size, gpu_buffer* out_gpu_buffer);
bool copy_to_upload_buffer(internal_vulkan_renderer_state* state, void* source, size_t size, gpu_buffer* buffer);
bool copy_to_gpu_buffer(VkCommandBuffer command_buffer, const gpu_buffer* source_upload_buffer, gpu_buffer* gpu_buffer);
bool create_gpu_buffer(const internal_vulkan_renderer_state* state, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, gpu_buffer* out_gpu_buffer);
bool destroy_gpu_buffer(const internal_vulkan_renderer_state* state, gpu_buffer* buffer);

bool create_one_time_command_buffer(const internal_vulkan_renderer_state* state, VkCommandBuffer* out_command_buffer);
bool end_one_time_command_buffer(const internal_vulkan_renderer_state* state, VkCommandBuffer command_buffer, VkQueue queue);

VkResult submit_command_queue(VkSemaphore wait_semaphore, VkSemaphore signal_semaphore, VkFence fence, VkQueue queue, VkCommandBuffer command_buffer);

bool create_mvp_pipeline_layout(internal_vulkan_renderer_state* state);
bool create_mvp_pipeline(internal_vulkan_renderer_state* state, const VkSurfaceCapabilitiesKHR& surface_capabilities);

}