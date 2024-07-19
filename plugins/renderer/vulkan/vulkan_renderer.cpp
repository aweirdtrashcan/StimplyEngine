#include "vulkan_renderer.h"

#include "containers/list.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "../render_item.h"
#include "vulkan_defines.h"
#include "renderer/render_item_utils.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

extern "C" {

struct internal_vulkan_renderer_state {
    VkInstance instance;
    SDL_Window* window;
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
    VkPipeline naked_graphics_pipeline;
    VkPipelineLayout naked_graphics_pipeline_layout;
    VkRenderPass main_renderpass;
    VkViewport viewport;
    VkRect2D scissor;
    list<VkFramebuffer> swapchain_framebuffers;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    list<VkClearValue> clear_values;
    list<render_item> render_items;
};

internal_vulkan_renderer_state* state = nullptr;

#define vk_result(result)                       \
    {                                           \
        VkResult res = (result);                \
        if (res != VK_SUCCESS) return false;    \
    }                                           \

/* internal functions */
static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData);
static bool create_vulkan_instance(const char* name);
static bool enumerate_required_instance_extensions(list<const char*>& out_required_extensions);
static bool enumerate_required_instance_layers(list<const char*>& out_required_layers);
static VkDebugUtilsMessengerCreateInfoEXT get_debug_utils_messenger_create_info();
static bool enable_validation_layer();
static bool select_physical_device();
static bool create_logical_device(const VkPhysicalDeviceFeatures* features);
static bool enumerate_required_device_extensions(list<const char*>& out_required_extensions);
static bool get_physical_device_queues();
static bool query_optimal_back_buffer_format(VkSurfaceFormatKHR* format);
static bool create_surface();
static bool create_swapchain(const VkSurfaceCapabilitiesKHR& surface_capabilities);
static bool destroy_swapchain();
static bool get_swapchain_back_buffers();
static bool create_swapchain_back_buffer_views();
static bool destroy_swapchain_image_views();
static bool create_fence(VkFence& out_fence, bool create_signaled);
static bool destroy_fence(VkFence fence);
static bool create_semaphore(VkSemaphore& out_semaphore);
static bool create_swapchain_semaphores_and_fences();
static bool destroy_swapchain_semaphores_and_fences();
/**
    Will create the depth VkImage, VkImageView, VkDeviceMemory, bind the memory, and assign the right format.
 */
static bool create_depth_buffer(const VkSurfaceCapabilitiesKHR& surface_capabilities);
static bool destroy_depth_buffer();
static bool find_memory_type_index(uint32_t supported_memory_type, VkMemoryPropertyFlags property_flags, uint32_t* out_memory_type_index);
static bool create_descriptor_pool(VkDescriptorType type, bool can_be_freed, VkDescriptorPool* out_descriptor_pool, uint32_t max_sets = 1000);
static bool destroy_descriptor_pool(VkDescriptorPool descriptor_pool);
static bool create_descriptor_set(VkDescriptorPool descriptor_pool, VkDescriptorSet* out_descriptor_set);
static bool free_descriptor_set(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
static bool create_command_pool(VkCommandPool* out_command_pool, uint32_t queueIndex);
static bool destroy_command_pool(VkCommandPool command_pool);
static bool allocate_command_buffers(VkCommandPool command_pool, uint32_t command_buffer_count, VkCommandBuffer* out_command_buffer);
static bool free_command_buffers(uint32_t command_buffer_count, VkCommandPool command_pool, VkCommandBuffer* command_buffers);
static bool create_render_pass();
static bool destroy_render_pass();
static bool create_naked_graphics_pipeline_state(const VkSurfaceCapabilitiesKHR& surface_capabilities);
static bool destroy_naked_graphics_pipeline_state();
static bool destroy_pipeline(VkPipeline pipeline);
static bool get_viewport_and_scissor(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkViewport* out_viewport, VkRect2D* out_scissor);
static bool create_shader_module(const char* shader_path, VkShaderModule* out_shader_module);
static bool destroy_shader_module(VkShaderModule shader_module);
static bool create_framebuffer(VkRenderPass renderpass, uint32_t attachment_count, VkImageView* attachments, uint32_t width, uint32_t height, VkFramebuffer* out_framebuffer);
static bool create_swapchain_framebuffers(const VkSurfaceCapabilitiesKHR& surface_capabilities);
static bool destroy_swapchain_framebuffers();
static bool recreate_swapchain();

static bool begin_command_buffer(VkCommandBuffer command_buffer);
static bool end_command_buffer(VkCommandBuffer command_buffer);
static bool begin_render_pass(VkRenderPass render_pass, VkCommandBuffer command_buffer, VkFramebuffer framebuffer, VkRect2D render_area, uint32_t clear_value_count, const VkClearValue* clear_values);
static bool end_render_pass(VkCommandBuffer command_buffer);
static bool create_uploader_buffer(size_t size, gpu_buffer* out_gpu_buffer);
static bool copy_to_upload_buffer(void* source, size_t size, gpu_buffer* buffer);
static bool copy_to_gpu_buffer(VkCommandBuffer command_buffer, const gpu_buffer* source_upload_buffer, gpu_buffer* gpu_buffer);
static bool create_gpu_buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, gpu_buffer* out_gpu_buffer);
static bool destroy_gpu_buffer(gpu_buffer* buffer);
static bool create_one_time_command_buffer(VkCommandBuffer* out_command_buffer);
static bool end_one_time_command_buffer(VkCommandBuffer command_buffer, VkQueue queue);
static VkResult submit_command_queue(VkSemaphore wait_semaphore, VkSemaphore signal_semaphore, VkFence fence, VkQueue queue, VkCommandBuffer command_buffer);

/* public functions */
bool vulkan_backend_initialize(uint64_t* required_size, void* allocated_memory, const char* name, void* sdl_window) {
    if (!required_size) return false;
    if (*required_size == 0) {
        *required_size = sizeof(internal_vulkan_renderer_state);
        return true;
    }
    if (!allocated_memory) {
        return false;
    }

    state = (internal_vulkan_renderer_state*)allocated_memory;

    Logger::info("Initializing Vulkan Backend");

    state->window = (SDL_Window*)sdl_window;

    if (!create_vulkan_instance(name)) {
        Logger::fatal("Failed to create vulkan instance");
        return false;
    }    

    if (!enable_validation_layer()) {
        Logger::fatal("Validaiton Layer was not enabled!");
    }

    if (!select_physical_device()) {
        Logger::fatal("Failed to select physical device");
        return false;
    }

    VkPhysicalDeviceFeatures features{};
    features.depthClamp = VK_TRUE;

    if (!create_logical_device(&features)) {
        Logger::fatal("Failed to create logical device");
        return false;
    }

    if (!get_physical_device_queues()) {
        Logger::fatal("Failed to get physical device queues");
        return false;
    }

    if (!create_surface()) {
        Logger::fatal("Failed to create surface");
        return false;
    }

    vk_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        state->physical_device, 
        state->surface, 
        &state->surface_capabilities));

    if (!create_swapchain(state->surface_capabilities)) {
        Logger::fatal("Failed to create swapchain");
        return false;
    }

    if (!get_swapchain_back_buffers()) {
        Logger::fatal("Failed to get swapchain backbuffers");
        return false;
    }

    if (!create_swapchain_back_buffer_views()) {
        Logger::fatal("Failed to create swapchain backbuffer views");
        return false;
    }

    if (!create_swapchain_semaphores_and_fences()) {
        Logger::fatal("Failed to create swapchain semaphores and fences");
        return false;
    }

    if (!create_depth_buffer(state->surface_capabilities)) {
        Logger::fatal("Failed to create depth buffer");
        return false;
    }

    if (!create_descriptor_pool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true, &state->uniform_descriptor_pool)) {
        Logger::fatal("Failed to create uniform buffer descriptor pool");
        return false;
    }

    if (!create_command_pool(&state->command_pool, state->graphics_queue_index)) {
        Logger::fatal("Failed to create graphics command pool");
        return false;
    }

    state->graphics_command_buffers.resize(state->num_frames);
    if (!allocate_command_buffers(state->command_pool, state->num_frames, state->graphics_command_buffers.data())) {
        Logger::fatal("Failed to allocate graphics command buffers");
        return false;
    }

    if (!create_render_pass()) {
        Logger::fatal("Failed to create main render pass");
        return false;
    }

    if (!create_naked_graphics_pipeline_state(state->surface_capabilities)) {
        Logger::fatal("Failed to create naked graphics pipeline");
        return false;
    }

    if (!create_swapchain_framebuffers(state->surface_capabilities)) {
        Logger::fatal("Failed to create swapchain framebuffers");
        return false;
    }

    VkClearValue color_clear_value{};
    color_clear_value.color.float32[0] = 0.8f;
    color_clear_value.color.float32[1] = 0.3f;
    color_clear_value.color.float32[2] = 0.8f;
    color_clear_value.color.float32[3] = 1.0f;

    VkClearValue depth_clear_value{};
    depth_clear_value.depthStencil.depth = 1.0f;

    state->clear_values.push_back(color_clear_value);
    state->clear_values.push_back(depth_clear_value);

    Logger::info("Vulkan Backend Initialized");

    return true;
}

void vulkan_backend_shutdown() {
    vkDeviceWaitIdle(state->logical_device);
    Logger::info("Shutting Down Vulkan Renderer");

    destroy_swapchain_framebuffers();
    destroy_naked_graphics_pipeline_state();
    destroy_render_pass();
    destroy_command_pool(state->command_pool);
    destroy_descriptor_pool(state->uniform_descriptor_pool);
    destroy_depth_buffer();
    destroy_swapchain_semaphores_and_fences();
    destroy_swapchain_image_views();
    destroy_swapchain();
    vkDestroySurfaceKHR(state->instance, state->surface, state->allocator);
    vkDestroyDevice(state->logical_device, state->allocator);

    if (state->messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func) {
            func(state->instance, state->messenger, state->allocator);
        }
    }

    vkDestroyInstance(state->instance, state->allocator);

    memset(state, 0, sizeof(*state));
}

bool vulkan_begin_frame() {
    VkFence fence = state->fences[state->current_frame_index];
    VkSemaphore queue_complete = state->queue_complete_semaphore[state->current_frame_index];
    VkSemaphore image_acquired = state->image_acquired_semaphore[state->current_frame_index];
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];
    VkRenderPass renderpass = state->main_renderpass;
    const VkSurfaceCapabilitiesKHR& surface_capabilities = state->surface_capabilities;
    const list<VkClearValue>& clear_values = state->clear_values;

    vk_result(vkWaitForFences(state->logical_device, 1, &fence, VK_TRUE, UINT64_MAX));

    VkResult result;

    result = vkAcquireNextImageKHR(
        state->logical_device, 
        state->swapchain, 
        UINT64_MAX, 
        image_acquired, 
        VK_NULL_HANDLE, 
        &state->image_index);

    if (result != VK_SUCCESS) {
        // TODO: Resize
        recreate_swapchain();
        return false;
    }

    vk_result(vkResetFences(state->logical_device, 1, &fence));

    VkFramebuffer framebuffer = state->swapchain_framebuffers[state->image_index];

    VkViewport viewport;
    VkRect2D scissor;
    get_viewport_and_scissor(surface_capabilities, &viewport, &scissor);

    vkResetCommandBuffer(command_buffer, 0);
    begin_command_buffer(command_buffer);

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    begin_render_pass(renderpass, command_buffer, framebuffer, scissor, (uint32_t)clear_values.size(), clear_values.data());

    return true;
}

bool vulkan_end_frame() {
    VkResult result;
    VkCommandBuffer command_buffer = state->graphics_command_buffers[state->current_frame_index];
    VkSemaphore image_acquired = state->image_acquired_semaphore[state->current_frame_index];
    VkSemaphore queue_completed = state->queue_complete_semaphore[state->current_frame_index];
    VkQueue queue = state->graphics_queue;
    VkFence fence = state->fences[state->current_frame_index];

    end_render_pass(command_buffer);
    end_command_buffer(command_buffer);

    result = submit_command_queue(image_acquired, queue_completed, fence, queue, command_buffer);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain();
        return false;
    } else if (result != VK_SUCCESS) {
        return false;
    }

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &queue_completed;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &state->swapchain;
    present_info.pImageIndices = &state->image_index;
    present_info.pResults = nullptr;

    //Logger::debug("Presenting built frame %u", state->current_frame_index);
    //Logger::debug("Presenting image %u", state->image_index);

    result = vkQueuePresentKHR(queue, &present_info);
    state->current_frame_index = (state->current_frame_index + 1) % state->num_frames;

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain();
        return false;
    } else if (result != VK_SUCCESS) {
        return false;
    }

    return true;
}

void* vulkan_create_render_item(const RenderItemCreateInfo* pRenderItemCreateInfo) {
    if (pRenderItemCreateInfo == nullptr) {
        Logger::debug("vulkan_create_render_item: pRenderItemCreateInfo can't be nullptr");
        return nullptr;
    }

    if (pRenderItemCreateInfo->pIndices == 0 || pRenderItemCreateInfo->pMeshes == 0) {
        Logger::debug("vulkan_create_render_item: RenderItemCreateInfo->pIndices and/or RenderItemCreateInfo->pMeshes can't be nullptr");
        return nullptr;
    }

    VkCommandBuffer command_buffer;
    create_one_time_command_buffer(&command_buffer);

    gpu_buffer vertex_uploader;
    gpu_buffer index_uploader;

    render_item* r_item = (render_item*)Platform::ualloc(sizeof(render_item));

    if (!create_uploader_buffer(pRenderItemCreateInfo->meshSize, &vertex_uploader)) {
        Logger::debug("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }

    if (!create_uploader_buffer(pRenderItemCreateInfo->indicesSize, &index_uploader)) {
        Logger::debug("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }

    if (!create_gpu_buffer(
        pRenderItemCreateInfo->meshSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &r_item->vertex_buffer)) {
        Logger::debug("vulkan_create_render_item: Failed to create vertex buffer");
        return nullptr;
    }

    if (!create_gpu_buffer(
        pRenderItemCreateInfo->indicesSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &r_item->index_buffer)) {
        Logger::debug("vulkan_create_render_item: Failed to create index buffer");
        return nullptr;
    }

    copy_to_upload_buffer(pRenderItemCreateInfo->pMeshes, pRenderItemCreateInfo->meshSize, &vertex_uploader);
    copy_to_upload_buffer(pRenderItemCreateInfo->pIndices, pRenderItemCreateInfo->indicesSize, &index_uploader);

    copy_to_gpu_buffer(command_buffer, &vertex_uploader, &r_item->vertex_buffer);
    copy_to_gpu_buffer(command_buffer, &index_uploader, &r_item->index_buffer);

    end_one_time_command_buffer(command_buffer, state->graphics_queue);

    destroy_gpu_buffer(&vertex_uploader);
    destroy_gpu_buffer(&index_uploader);

    return r_item;
}

void vulkan_destroy_render_item(void* r_item) {
    destroy_gpu_buffer(&(((render_item*)r_item))->vertex_buffer);
    destroy_gpu_buffer(&(((render_item*)r_item))->index_buffer);
    Platform::zero_memory(r_item, sizeof(render_item));
    Platform::ufree(r_item);
}

/****************************************** INTERNAL FUNCTIONS ********************************************* */
static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData) {

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Logger::debug("%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Logger::warning("%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Logger::info("%s", pCallbackData->pMessage);
        default:
            Logger::info("%s", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

bool create_vulkan_instance(const char* name) {
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = name;
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "Stimply Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    list<const char*> extension_properties;

    if (!enumerate_required_instance_extensions(extension_properties)) {
        Logger::fatal("One or more instance extensions are not supported!");
        return false;
    }

    list<const char*> layer_properties;

    if (!enumerate_required_instance_layers(layer_properties)) {
        Logger::fatal("One or more instance layers are not supported!");
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = get_debug_utils_messenger_create_info();

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = &debug_create_info;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = static_cast<uint32_t>(layer_properties.size());
    instance_create_info.ppEnabledLayerNames = layer_properties.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extension_properties.size());
    instance_create_info.ppEnabledExtensionNames = extension_properties.data();

    if (vkCreateInstance(&instance_create_info, state->allocator, &state->instance) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool enumerate_required_instance_extensions(list<const char*>& out_required_extensions) {
    list<VkExtensionProperties> supported_extensions;
    uint32_t supported_extension_count = 0;
    
    vk_result(vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, nullptr));

    supported_extensions.resize(supported_extension_count);

    vk_result(vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, supported_extensions.data()));

    for (const VkExtensionProperties& extension : supported_extensions) {
        Logger::debug("Supported Instance Extensions: %s", extension.extensionName);
    }

    uint32_t required_extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(state->window, &required_extensions_count, nullptr);
    out_required_extensions.resize(required_extensions_count);
    SDL_Vulkan_GetInstanceExtensions(state->window, &required_extensions_count, out_required_extensions.data());

#ifdef DEBUG
    out_required_extensions.push_back("VK_EXT_debug_utils");
#endif

    for (uint32_t i = 0; i < (uint32_t)out_required_extensions.size(); i++) {
        bool found = false;

        for (uint32_t f = 0; f < supported_extension_count; f++) {
            if (strcmp(out_required_extensions[i], supported_extensions[f].extensionName) == 0) {
                found = true;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

bool enumerate_required_instance_layers(list<const char*>& out_required_layers) {
    list<VkLayerProperties> supported_layers;
    uint32_t supported_layer_count = 0;

    vk_result(vkEnumerateInstanceLayerProperties(&supported_layer_count, nullptr));

    supported_layers.resize(supported_layer_count);

    vk_result(vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers.data()));

    for (const VkLayerProperties& property : supported_layers) {
        Logger::debug("Supported Instance Layers: %s", property.layerName);
    }

    const char* validation = "VK_LAYER_KHRONOS_validation";

    for (uint32_t i = 0; i < supported_layer_count; i++) {
#ifdef DEBUG
        if (strcmp(supported_layers[i].layerName, validation) == 0) {
            out_required_layers.push_back(validation);
        }
#endif
    }

    return true;
}

static VkDebugUtilsMessengerCreateInfoEXT get_debug_utils_messenger_create_info() {
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    create_info.pfnUserCallback = debug_utils_callback;
    create_info.pUserData = nullptr;

    return create_info;
}

bool enable_validation_layer() {
#ifdef DEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkCreateDebugUtilsMessengerEXT");

    if (!func) return false;

    VkDebugUtilsMessengerCreateInfoEXT create_info = get_debug_utils_messenger_create_info();
    vk_result(func(state->instance, &create_info, state->allocator, &state->messenger));
#endif
    return true;
}

bool select_physical_device() {
    uint32_t physical_device_count = 0;
    vk_result(vkEnumeratePhysicalDevices(state->instance, &physical_device_count, nullptr));
    VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)malloc(physical_device_count);
    vk_result(vkEnumeratePhysicalDevices(state->instance, &physical_device_count, physical_devices));

    // TODO: Better algorithm for GPU Detection
    size_t vram_count = 0;
    VkPhysicalDevice selected_device = nullptr;
    VkPhysicalDeviceProperties selected_physical_device;

    for (uint32_t i = 0; i < physical_device_count; i++) {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        VkPhysicalDeviceFeatures device_features;

        vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory_properties);
        vkGetPhysicalDeviceFeatures(physical_devices[i], &device_features);

        size_t device_vram = 0;
        for (uint32_t h = 0; h < memory_properties.memoryHeapCount; h++) {
            device_vram += memory_properties.memoryHeaps[h].size;
        }

        if (device_vram > vram_count) {
            selected_device = physical_devices[i];
            selected_physical_device = device_properties;
        }
    }

    Logger::info("Selecting Physical Device: ", selected_physical_device.deviceName);

    if (!selected_device) {
        return false;
    }

    state->physical_device = selected_device;

    return true;
}

bool create_logical_device(const VkPhysicalDeviceFeatures* features) {
    float priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = nullptr;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = state->graphics_queue_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = priority;

    list<const char*> required_extensions;

    if (!enumerate_required_device_extensions(required_extensions)) {
        Logger::fatal("Failed to acquire required device extension");
        return false;
    }

    VkDeviceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.pEnabledFeatures = features;

    vk_result(vkCreateDevice(state->physical_device, &create_info, state->allocator, &state->logical_device));

    return true;
}

bool enumerate_required_device_extensions(list<const char*>& out_required_extensions) {
    uint32_t supported_extension_count = 0;
    list<VkExtensionProperties> supported_extensions;

    vk_result(vkEnumerateDeviceExtensionProperties(state->physical_device, nullptr, &supported_extension_count, nullptr));
    
    supported_extensions.resize(supported_extension_count);

    vk_result(vkEnumerateDeviceExtensionProperties(state->physical_device, nullptr, &supported_extension_count, supported_extensions.data()));

    for (const VkExtensionProperties& property : supported_extensions) {
        Logger::debug("Supported Device Extensions: %s", property.extensionName);
    }

    list<const char*> requested_extensions(2);
    requested_extensions.push_back("VK_KHR_maintenance1");
    requested_extensions.push_back("VK_KHR_swapchain");
    //requested_extensions.push_back("VK_KHR_pipeline_library");
    //requested_extensions.push_back("VK_EXT_graphics_pipeline_library");

    for (const char* requested_extension : requested_extensions) {
        bool supported = false;
        for (uint32_t j = 0; j < supported_extension_count; j++) {
            if (strcmp(requested_extension, supported_extensions[j].extensionName) == 0) {
                supported = true;
                break;
            }
        }

        if (!supported) {
            Logger::fatal("Extension %s is not supported by your GPU!", requested_extension);
            return false;
        }
    }

    out_required_extensions = requested_extensions;

    return true;
}

bool get_physical_device_queues() {
    state->graphics_queue_index = UINT32_MAX;
    state->graphics_queue = 0;

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(state->physical_device, &queue_count, nullptr);

    VkQueueFamilyProperties* family_properties = (VkQueueFamilyProperties*)malloc(queue_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(state->physical_device, &queue_count, family_properties);

    for (uint32_t i = 0; i < queue_count; i++) {
        if (family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            state->graphics_queue_index = i;
        }
    }

    if (state->graphics_queue_index == UINT32_MAX) {
        return false;
    }

    vkGetDeviceQueue(state->logical_device, state->graphics_queue_index, 0, &state->graphics_queue);

    if (state->graphics_queue == 0) {
        return false;
    }

    return true;
}

bool create_surface() {
    SDL_Vulkan_CreateSurface(state->window, state->instance, &state->surface);

    if (state->surface == nullptr) {
        return false;
    }

    return true;
}

bool query_optimal_back_buffer_format(VkSurfaceFormatKHR* format) {
    if (format == nullptr) return false;

    uint32_t format_count;
    
    vk_result(vkGetPhysicalDeviceSurfaceFormatsKHR(
        state->physical_device, 
        state->surface, 
        &format_count, 
        nullptr));

    list<VkSurfaceFormatKHR> surface_format;
    surface_format.resize(format_count);

    vk_result(vkGetPhysicalDeviceSurfaceFormatsKHR(
        state->physical_device, 
        state->surface, 
        &format_count, 
        surface_format.data()));

    for (VkSurfaceFormatKHR supported_format : surface_format) {
        if (supported_format.format == VK_FORMAT_B8G8R8A8_UNORM && supported_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            *format = supported_format;
            return true;
        }
    }

    return false;
}

bool create_swapchain(const VkSurfaceCapabilitiesKHR& surface_capabilities) {   
    if (surface_capabilities.currentExtent.width == 0 || surface_capabilities.currentExtent.height == 0) {
        return false;
    }

    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.surface = state->surface;

    uint32_t min_image_count = surface_capabilities.minImageCount;

    // ask for at least double-buffer
    if (surface_capabilities.minImageCount < 2) {
        min_image_count = 2;
    }

    create_info.minImageCount = min_image_count;
    
    VkSurfaceFormatKHR surface_format;
    if (!query_optimal_back_buffer_format(&surface_format)) {
        return false;
    }

    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = surface_capabilities.currentExtent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    vk_result(vkCreateSwapchainKHR(state->logical_device, &create_info, state->allocator, &state->swapchain));

    state->back_buffer_format = surface_format.format;
    // TODO: Multisampling?
    state->samples = VK_SAMPLE_COUNT_1_BIT;
    // TODO: Mip-mapping?
    state->mip_levels = 1;

    return true;
}

bool destroy_swapchain() {
    vkDestroySwapchainKHR(state->logical_device, state->swapchain, state->allocator);

    return true;
}

bool get_swapchain_back_buffers() {
    state->num_frames = 0;
    vk_result(vkGetSwapchainImagesKHR(state->logical_device, state->swapchain, &state->num_frames, nullptr));
    state->back_buffers.resize(state->num_frames);
    vk_result(vkGetSwapchainImagesKHR(state->logical_device, state->swapchain, &state->num_frames, state->back_buffers.data()));

    return true;
}

bool create_swapchain_back_buffer_views() {
    state->back_buffer_views.resize(state->num_frames);

    for (uint32_t i = 0; i < state->num_frames; i++) {
        VkImageViewCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.image = state->back_buffers[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = state->back_buffer_format;
        // 0 = identity
        memset(&create_info.components, 0, sizeof(create_info.components));
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.layerCount = 1;
        create_info.subresourceRange.levelCount = 1;

        vk_result(vkCreateImageView(state->logical_device, &create_info, state->allocator, &state->back_buffer_views[i]));
    }

    return true;
}

bool destroy_swapchain_image_views() {
    for (VkImageView view : state->back_buffer_views) {
        vkDestroyImageView(state->logical_device, view, state->allocator);
    }

    return true;
}

bool create_fence(VkFence& out_fence, bool create_signaled) {
    VkFenceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = create_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    vk_result(vkCreateFence(state->logical_device, &create_info, state->allocator, &out_fence));

    return true;
}

bool destroy_fence(VkFence fence) {
    vkDestroyFence(state->logical_device, fence, state->allocator);

    return true;
}

bool create_semaphore(VkSemaphore& out_semaphore) {
    VkSemaphoreCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;

    vk_result(vkCreateSemaphore(state->logical_device, &create_info, state->allocator, &out_semaphore));

    return true;
}

bool create_swapchain_semaphores_and_fences() {
    state->fences.remove_all();
    state->last_image_fence.remove_all();
    state->image_acquired_semaphore.remove_all();
    state->queue_complete_semaphore.remove_all();

    state->fences.resize(state->num_frames);
    state->image_acquired_semaphore.resize(state->num_frames);
    state->queue_complete_semaphore.resize(state->num_frames);
    state->last_image_fence.resize(state->num_frames);
    memset(state->last_image_fence.data(), 0, sizeof(VkFence) * state->num_frames);

    for (uint32_t i = 0; i < state->num_frames; i++) {
        if (!create_fence(state->fences[i], true)) {
            return false;
        }
        if (!create_semaphore(state->image_acquired_semaphore[i])) {
            return false;
        }
        if (!create_semaphore(state->queue_complete_semaphore[i])) {
            return false;
        }
    }

    return true;
}

bool destroy_swapchain_semaphores_and_fences() {
    for (VkFence fence : state->fences) {
        vkDestroyFence(state->logical_device, fence, state->allocator);
    }
    for (VkSemaphore semaphore : state->image_acquired_semaphore) {
        vkDestroySemaphore(state->logical_device, semaphore, state->allocator);
    }
    for (VkSemaphore semaphore : state->queue_complete_semaphore) {
        vkDestroySemaphore(state->logical_device, semaphore, state->allocator);
    }

    return true;
}

bool create_depth_buffer(const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    state->depth_buffer.format = VK_FORMAT_D24_UNORM_S8_UINT;
    state->depth_buffer.width = surface_capabilities.currentExtent.width;
    state->depth_buffer.height = surface_capabilities.currentExtent.height;

    VkImageCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = state->depth_buffer.format;
    create_info.extent.width = state->depth_buffer.width;
    create_info.extent.height = state->depth_buffer.height;
    create_info.extent.depth = 1;
    create_info.mipLevels = state->mip_levels;
    create_info.arrayLayers = 1;
    create_info.samples = state->samples;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vk_result(vkCreateImage(state->logical_device, &create_info, state->allocator, &state->depth_buffer.image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state->logical_device, state->depth_buffer.image, &memory_requirements);

    uint32_t memory_index; 
    if (!find_memory_type_index(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_index)) {
        return false;
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_index;
    
    vk_result(vkAllocateMemory(state->logical_device, &allocate_info, state->allocator, &state->depth_buffer.memory));

    vk_result(vkBindImageMemory(state->logical_device, state->depth_buffer.image, state->depth_buffer.memory, 0));
    
    VkImageViewCreateInfo view_create_info;
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.pNext = nullptr;
    view_create_info.flags = 0;
    view_create_info.image = state->depth_buffer.image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = state->depth_buffer.format;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    vk_result(vkCreateImageView(state->logical_device, &view_create_info, state->allocator, &state->depth_buffer.view));

    return true;
}

bool destroy_depth_buffer() {
    if (state->depth_buffer.memory) {
        vkFreeMemory(state->logical_device, state->depth_buffer.memory, state->allocator);
    }
    if (state->depth_buffer.view) {
        vkDestroyImageView(state->logical_device, state->depth_buffer.view, state->allocator);
    }
    if (state->depth_buffer.image) {
        vkDestroyImage(state->logical_device, state->depth_buffer.image, state->allocator);
    }

    memset(&state->depth_buffer, 0, sizeof(state->depth_buffer));

    return true;
}

bool find_memory_type_index(uint32_t supported_memory_type, VkMemoryPropertyFlags property_flags, uint32_t* out_memory_type_index) {
    if (out_memory_type_index == nullptr) {
        Logger::fatal("find_memory_type_index: out_memory_type_index can't be nullptr");
        return false;
    }

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(state->physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        bool is_type_supported = (supported_memory_type & (1 << i));
        bool is_flag_supported = (property_flags & mem_properties.memoryTypes[i].propertyFlags) == property_flags;

        if (is_type_supported && is_flag_supported) {
            *out_memory_type_index = i;
            return true;
        }
    }

    return false;
}

bool create_descriptor_pool(VkDescriptorType type, bool can_be_freed, VkDescriptorPool* out_descriptor_pool, uint32_t max_sets) {
    if (out_descriptor_pool == nullptr) {
        Logger::fatal("create_descriptor_pool: out_descriptor_pool can't be nullptr");
        return false;
    }

    VkDescriptorPoolSize pool_size;
    pool_size.type = type;
    pool_size.descriptorCount = max_sets;

    VkDescriptorPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = can_be_freed ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;

    vk_result(vkCreateDescriptorPool(state->logical_device, &create_info, state->allocator, out_descriptor_pool));

    return true;
}

bool destroy_descriptor_pool(VkDescriptorPool descriptor_pool) {
    vkDestroyDescriptorPool(state->logical_device, descriptor_pool, state->allocator);
    return true;
}

bool create_descriptor_set(VkDescriptorPool descriptor_pool, VkDescriptorSet* out_descriptor_set) {
    if (out_descriptor_set == nullptr) {
        Logger::fatal("create_descriptor_set: out_descriptor_pool can't be nullptr");
        return false;
    }
    VkDescriptorSetAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.descriptorPool = descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    
    vk_result(vkAllocateDescriptorSets(state->logical_device, &allocate_info, out_descriptor_set));

    return true;
}

bool create_command_pool(VkCommandPool* out_command_pool, uint32_t queueIndex) {
    if (out_command_pool == nullptr) {
        Logger::fatal("create_command_pool: out_command_pool can't be nullptr");
        return false;
    }

    VkCommandPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.queueFamilyIndex = queueIndex;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vk_result(vkCreateCommandPool(state->logical_device, &create_info, state->allocator, out_command_pool));

    return true;
}

bool destroy_command_pool(VkCommandPool command_pool) {
    vkDestroyCommandPool(state->logical_device, command_pool, state->allocator);
    return true;
}

bool allocate_command_buffers(VkCommandPool command_pool, uint32_t command_buffer_count, VkCommandBuffer* out_command_buffer) {
    VkCommandBufferAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = command_buffer_count;

    vk_result(vkAllocateCommandBuffers(state->logical_device, &allocate_info, out_command_buffer));

    return true;
}

bool free_command_buffers(uint32_t command_buffer_count, VkCommandPool command_pool, VkCommandBuffer* command_buffers) {
    vkFreeCommandBuffers(state->logical_device, command_pool, command_buffer_count, command_buffers);
    return true;
}

bool create_render_pass() {
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

bool destroy_render_pass() {
    vkDestroyRenderPass(state->logical_device, state->main_renderpass, state->allocator);
    state->main_renderpass = nullptr;
    return true;
}

bool create_naked_graphics_pipeline_state(const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    if (state->naked_graphics_pipeline || state->naked_graphics_pipeline_layout) {
        Logger::fatal("create_naked_graphics_pipeline_state: either the pipeline layout or the pipeline state are already created");
        return false;
    }

    {
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
            &state->naked_graphics_pipeline_layout));
    }

    {
        VkShaderModule vertex_module;
        VkShaderModule fragment_module;

        create_shader_module("./naked_vert.spv", &vertex_module);
        create_shader_module("./naked_frag.spv", &fragment_module);

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
        create_info.layout = state->naked_graphics_pipeline_layout;
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
            &state->naked_graphics_pipeline));

        destroy_shader_module(vertex_module);
        destroy_shader_module(fragment_module);
    }

    return true;
}

bool destroy_naked_graphics_pipeline_state() {
    vkDestroyPipeline(state->logical_device, state->naked_graphics_pipeline, state->allocator);
    vkDestroyPipelineLayout(state->logical_device, state->naked_graphics_pipeline_layout, state->allocator);

    return true;
}

bool get_viewport_and_scissor(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkViewport* out_viewport, VkRect2D* out_scissor) {
    float width = (float)surface_capabilities.currentExtent.width;
    float height = (float)surface_capabilities.currentExtent.height;

    out_viewport->x = 0.0f;
    out_viewport->y = height;
    out_viewport->width = width;
    out_viewport->height = -height;
    out_viewport->minDepth = 0.0f;
    out_viewport->maxDepth = 1.0f;

    out_scissor->extent = surface_capabilities.currentExtent;
    out_scissor->offset = {};

    return true;
}

static bool create_shader_module(const char* shader_path, VkShaderModule* out_shader_module) {
    binary_info shader = Platform::read_binary(shader_path);
    
    if (shader.size) {
        VkShaderModuleCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.codeSize = shader.size;
        create_info.pCode = (uint32_t*)shader.binary;

        vk_result(vkCreateShaderModule(state->logical_device, &create_info, state->allocator, out_shader_module));

        return true;
    }

    return false;
}

static bool destroy_shader_module(VkShaderModule shader_module) {
    vkDestroyShaderModule(state->logical_device, shader_module, state->allocator);
    return true;
}

bool create_framebuffer(VkRenderPass renderpass, uint32_t attachment_count, VkImageView* attachments, uint32_t width, uint32_t height, VkFramebuffer* out_framebuffer) {
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

bool create_swapchain_framebuffers(const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    state->swapchain_framebuffers.resize(state->num_frames);
    for (uint32_t i = 0; i < state->num_frames; i++) {
        VkImageView attachments[] = {
            state->back_buffer_views[i],
            state->depth_buffer.view
        };

        if (!create_framebuffer(
                state->main_renderpass, 
                std::size(attachments), 
                attachments, 
                state->surface_capabilities.currentExtent.width,
                state->surface_capabilities.currentExtent.height,
                &state->swapchain_framebuffers[i])) {
            return false;
        }
    }

    return true;
}

bool destroy_swapchain_framebuffers() {
    for (uint32_t i = 0; i < state->num_frames; i++) {
        vkDestroyFramebuffer(state->logical_device, state->swapchain_framebuffers[i], state->allocator);
    }

    return true;
}

static bool begin_command_buffer(VkCommandBuffer command_buffer) {
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    vk_result(vkBeginCommandBuffer(command_buffer, &begin_info));

    return true;
}

static bool end_command_buffer(VkCommandBuffer command_buffer) {
    vkEndCommandBuffer(command_buffer);
    return true;
}

static bool begin_render_pass(VkRenderPass render_pass, VkCommandBuffer command_buffer, VkFramebuffer framebuffer, VkRect2D render_area, uint32_t clear_value_count, const VkClearValue* clear_values) {
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

static bool end_render_pass(VkCommandBuffer command_buffer) {
    vkCmdEndRenderPass(command_buffer);
    return true;
}

bool recreate_swapchain() {
    vkDeviceWaitIdle(state->logical_device);

    destroy_swapchain_framebuffers();
    destroy_render_pass();
    destroy_swapchain_image_views();
    destroy_swapchain();
    destroy_depth_buffer();
    state->swapchain = nullptr;

    vk_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        state->physical_device, 
        state->surface, 
        &state->surface_capabilities));

    if (!create_swapchain(state->surface_capabilities)) {
        return false;
    }
    if (!get_swapchain_back_buffers()) {
        return false;
    }
    if (!create_swapchain_back_buffer_views()) {
        return false;
    }
    if (!create_depth_buffer(state->surface_capabilities)) {
        return false;
    }
    if (!create_render_pass()) {
        return false;
    }
    if (!create_swapchain_framebuffers(state->surface_capabilities)) {
        return false;
    }

    return true;    
}

bool create_uploader_buffer(size_t size, gpu_buffer* out_gpu_buffer) {
    if (out_gpu_buffer == nullptr) {
        Logger::fatal("create_uploader_buffer: out_gpu_buffer can't be nullptr");
        return false;
    }

    Platform::zero_memory(out_gpu_buffer, sizeof(*out_gpu_buffer));

    VkBufferCreateInfo buffer_create;
    buffer_create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create.pNext = nullptr;
    buffer_create.flags = 0;
    buffer_create.size = size;
    buffer_create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create.queueFamilyIndexCount = 0;
    buffer_create.pQueueFamilyIndices = nullptr;
    
    vk_result(vkCreateBuffer(state->logical_device, &buffer_create, state->allocator, &out_gpu_buffer->buffer));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(state->logical_device, out_gpu_buffer->buffer, &memory_requirements);

    uint32_t memory_index;
    if (!find_memory_type_index(
        memory_requirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
        &memory_index)) {
        return false;    
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_index;

    vk_result(vkAllocateMemory(state->logical_device, &allocate_info, state->allocator, &out_gpu_buffer->memory));

    vk_result(vkBindBufferMemory(state->logical_device, out_gpu_buffer->buffer, out_gpu_buffer->memory, 0));

    out_gpu_buffer->size = memory_requirements.size;

    vk_result(vkMapMemory(state->logical_device, out_gpu_buffer->memory, 0, out_gpu_buffer->size, 0, &out_gpu_buffer->memory_pointer));

    return true;
}

bool copy_to_upload_buffer(void* source, size_t size, gpu_buffer* buffer) {
    memcpy(buffer->memory_pointer, source, size);

    VkMappedMemoryRange memory_range;
    memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memory_range.pNext = nullptr;
    memory_range.memory = buffer->memory;
    memory_range.offset = 0;
    memory_range.size = buffer->size;

    vk_result(vkFlushMappedMemoryRanges(state->logical_device, 1, &memory_range));
    
    return true;
}

bool copy_to_gpu_buffer(VkCommandBuffer command_buffer, const gpu_buffer* source_upload_buffer, gpu_buffer* gpu_buffer) {
    VkBufferCopy region;
    region.srcOffset = 0;
    region.size = source_upload_buffer->size;
    region.dstOffset = 0;

    vkCmdCopyBuffer(command_buffer, source_upload_buffer->buffer, gpu_buffer->buffer, 1, &region);
    return true;
}

bool create_gpu_buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, gpu_buffer* out_gpu_buffer) {
    if (out_gpu_buffer == nullptr) {
        Logger::fatal("create_gpu_buffer: out_gpu_buffer can't be nullptr");
        return false;
    }

    Platform::zero_memory(out_gpu_buffer, sizeof(*out_gpu_buffer));

    VkBufferCreateInfo buffer_create;
    buffer_create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create.pNext = nullptr;
    buffer_create.flags = 0;
    buffer_create.size = size;
    buffer_create.usage = usage;
    buffer_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create.queueFamilyIndexCount = 0;
    buffer_create.pQueueFamilyIndices = nullptr;
    
    VkResult res = vkCreateBuffer(state->logical_device, &buffer_create, state->allocator, &out_gpu_buffer->buffer);

    if (res != VK_SUCCESS) {
        Logger::debug("create_gpu_buffer: Failed to create vulkan buffer");
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(state->logical_device, out_gpu_buffer->buffer, &memory_requirements);

    uint32_t memory_index;
    if (!find_memory_type_index(
        memory_requirements.memoryTypeBits, 
        memory_properties, 
        &memory_index)) {
        Logger::debug("create_gpu_buffer: Failed to find memory type index");
        return false;    
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_index;

    res = vkAllocateMemory(state->logical_device, &allocate_info, state->allocator, &out_gpu_buffer->memory);

    if (res != VK_SUCCESS) {
        Logger::debug("create_gpu_buffer: Failed to allocate buffer memory");
        goto cleanup;
    }

    res = vkBindBufferMemory(state->logical_device, out_gpu_buffer->buffer, out_gpu_buffer->memory, 0);

    if (res != VK_SUCCESS) {
        Logger::debug("create_gpu_buffer: Failed to bind buffer memory");
        goto cleanup;
    }

    out_gpu_buffer->size = memory_requirements.size;

    return true;

cleanup:
    if (out_gpu_buffer->buffer) {
        vkDestroyBuffer(state->logical_device, out_gpu_buffer->buffer, state->allocator);
    }
    if (out_gpu_buffer->memory) {
        vkFreeMemory(state->logical_device, out_gpu_buffer->memory, state->allocator);
    }

    Platform::zero_memory(out_gpu_buffer, sizeof(*out_gpu_buffer));

    return false;
}

bool destroy_gpu_buffer(gpu_buffer* buffer) {
    vkDestroyBuffer(state->logical_device, buffer->buffer, state->allocator);
    vkFreeMemory(state->logical_device, buffer->memory, state->allocator);
    Platform::zero_memory(buffer, sizeof(*buffer));

    return true;
}

bool create_one_time_command_buffer(VkCommandBuffer* out_command_buffer) {
    if (!allocate_command_buffers(state->command_pool, 1, out_command_buffer)) {
        return false;
    }

    begin_command_buffer(*out_command_buffer);

    return true;
}

bool end_one_time_command_buffer(VkCommandBuffer command_buffer, VkQueue queue) {
    end_command_buffer(command_buffer);
    
    VkFence fence;
    create_fence(fence, false);

    submit_command_queue(nullptr, nullptr, fence, queue, command_buffer);
    
    vk_result(vkWaitForFences(state->logical_device, 1, &fence, VK_TRUE, UINT64_MAX));

    free_command_buffers(1, state->command_pool, &command_buffer);

    destroy_fence(fence);

    return true;
}

VkResult submit_command_queue(VkSemaphore wait_semaphore, VkSemaphore signal_semaphore, VkFence fence, VkQueue queue, VkCommandBuffer command_buffer) {
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
    submit_info.pWaitSemaphores = &wait_semaphore;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = signal_semaphore ? 1 : 0;
    submit_info.pSignalSemaphores = &signal_semaphore;

    return vkQueueSubmit(queue, 1, &submit_info, fence);
}

}
