#ifdef PLATFORM_WINDOWS
#include "d3d12_renderer.h"

#include "containers/list.h"
#include "core/logger.h"
#include "renderer/renderer_exception.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <wrl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <DirectXMath.h>
#include <DirectXMath/Extensions/DirectXMathAVX2.h>

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;

struct internal_d3d12_renderer_state {
    internal_d3d12_renderer_state() = default;
    ~internal_d3d12_renderer_state() = default;

    ComPtr<ID3D12Device1> device;
    ComPtr<IDXGIFactory2> factory;
    ComPtr<IDXGIAdapter1> adapter;
    bool has_debug_layer_enabled;
    ComPtr<IDXGIInfoQueue> info_queue;
    SDL_Window* window;
    int32_t width;
    int32_t height;
    ComPtr<ID3D12CommandQueue> graphics_queue;
    DXGI_FORMAT back_buffer_format;
    DXGI_FORMAT depth_stencil_format;
    ComPtr<IDXGISwapChain3> swapchain;
    uint32_t num_frames;
    list<ComPtr<ID3D12Resource>> back_buffers;
    uint64_t render_target_view_heap_step_size;
    uint64_t constant_buffer_view_heap_step_size;
    uint64_t sampler_heap_step_size;
    uint64_t depth_stencil_view_heap_step_size;
    ComPtr<ID3D12DescriptorHeap> render_target_view_heap;
    ComPtr<ID3D12DescriptorHeap> depth_stencil_view_heap;
    ComPtr<ID3D12DescriptorHeap> constant_buffer_heap;
    list<ComPtr<ID3D12Fence>> fences;
    list<uint64_t> fence_values;
    uint32_t mip_levels;
    ComPtr<ID3D12Resource> depth_buffer;
    list<ComPtr<ID3D12CommandAllocator>> graphics_command_allocator;
    list<ComPtr<ID3D12GraphicsCommandList>> graphics_command_lists;
    ComPtr<ID3D12PipelineState> naked_graphics_pipeline;
    ComPtr<ID3D12RootSignature> naked_root_signature;
    uint32_t frame_num;
    
};

extern "C" {

#define d3d12_res(result)                   \
    {                                       \
        HRESULT res = (result);             \
        poll_for_d3d12_messages();          \
        if (FAILED(res)) return false;      \
    }

#define d3d12_infocheck(call) { (call); bool had_error = poll_for_d3d12_messages(); if (had_error) return false; }

internal_d3d12_renderer_state* state;
/* internal functions */
static void __stdcall dxgi_message_func(DXGI_INFO_QUEUE_MESSAGE_SEVERITY Severity, LPCSTR pDescription);
static bool poll_for_d3d12_messages();
static bool create_dxgi_factory();
static bool create_d3d12_device();
static bool enable_debug_layer();
static bool create_device_queues();
static bool create_swapchain(uint32_t width, uint32_t height);
static bool get_swapchain_back_buffers();
static bool get_descriptor_heap_increment_sizes();
static bool create_descriptor_heap(ID3D12DescriptorHeap** out_heap, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptor);
static bool create_render_target_views();
static bool create_fence(ID3D12Fence** out_fence);
static bool create_swapchain_fences();
static bool create_depth_buffer(uint32_t width, uint32_t height);
static bool create_depth_view();
static bool create_command_allocator(ID3D12CommandAllocator** out_allocator, D3D12_COMMAND_LIST_TYPE type);
static bool create_command_list(void** out_command_list, ID3D12CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type);
static bool create_graphics_command_allocator_and_lists();
static bool create_naked_pipeline_state_object();
static bool _temp_get_shader(ComPtr<ID3DBlob>& blob);
static bool get_viewport_and_scissor(D3D12_VIEWPORT* out_viewport, D3D12_RECT* out_scissor);
static bool resource_transition(ID3D12GraphicsCommandList* command_list, ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after);
static bool signal_fence(ID3D12CommandQueue* queue, ID3D12Fence* fence, uint64_t* value);
static bool wait_for_fence(ID3D12Fence* fence, uint64_t value_to_wait);
static bool wait_device_idle();

/* public functions */
bool d3d12_backend_initialize(uint64_t* required_size, void* allocated_memory, const char* name, void* sdl_window) noexcept(false) {
    if (required_size == nullptr) return false;
    if (*required_size == 0) {
        *required_size = sizeof(internal_d3d12_renderer_state);
        return true;
    }
    if (!allocated_memory) {
        return false;
    }

    // since i'm using instances (ComPtr), i'll call the constructor.
    // as of now this memory is also zeroed out.
    state = new(allocated_memory) internal_d3d12_renderer_state;

    Logger::info("Initializing D3D12 Backend");

    if (!DirectX::XMVerifyCPUSupport()) {
        throw RendererException("Your CPU does not support SSE Instruction Set, which is required by this application.");
    }

    if (!DirectX::AVX2::XMVerifyAVX2Support()) {
        throw RendererException("Your CPU does not support AVX2 Instruction Set, which is required by this application.");
    }

    state->window = (SDL_Window*)sdl_window;

    if (!enable_debug_layer()) {
        Logger::debug("Failed to enable d3d12 debug layer");
    }

    if (!create_dxgi_factory()) {
        Logger::fatal("Failed to create DXGI Factory");
        return false;
    }

    if (!create_d3d12_device()) {
        Logger::fatal("Failed to create d3d12 Device");
        return false;
    }

    if (!create_device_queues()) {
        Logger::fatal("Failed to create d3d12 graphics queue");
        return false;
    }

    SDL_GetWindowSize(state->window, &state->width, &state->height);
    if (!create_swapchain(state->width, state->height)) {
        Logger::fatal("Failed to create D3D12 swapchain");
        return false;
    }

    if (!get_swapchain_back_buffers()) {
        Logger::fatal("Failed to get swapchain backbuffers");
        return false;
    }

    get_descriptor_heap_increment_sizes();

    if (!create_descriptor_heap(&state->render_target_view_heap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, state->num_frames)) {
        Logger::fatal("Failed to create render target view heap");
        return false;
    }

    if (!create_descriptor_heap(&state->depth_stencil_view_heap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1)) {
        Logger::fatal("Failed to create depth stencil view heap");
        return false;
    }

    if (!create_descriptor_heap(&state->constant_buffer_heap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1000)) {
        Logger::fatal("Failed to create constant buffer heap");
        return false;
    }

    if (!create_render_target_views()) {
        Logger::fatal("Failed to create render target views");
        return false;
    }

    if (!create_swapchain_fences()) {
        Logger::fatal("Failed to create swapchain fences");
        return false;
    }

    if (!create_depth_buffer(state->width, state->height)) {
        Logger::fatal("Failed to create depth buffer");
        return false;
    }
    
    if (!create_depth_view()) {
        Logger::fatal("Failed to create depth stencil view");
        return false;
    }

    if (!create_graphics_command_allocator_and_lists()) {
        Logger::fatal("Failed to create swapchain command lists and/or allocators");
        return false;
    }

    if (!create_naked_pipeline_state_object()) {
        Logger::fatal("Failed to create naked pipeline state object");
        return false;
    }

    Logger::info("D3D12 Backend Initialized");

    return true;
}

void d3d12_backend_shutdown() {
    Logger::info("Shutting Down D3D12 Renderer");
    wait_device_idle();
    state->~internal_d3d12_renderer_state();
    memset(state, 0, sizeof(*state));
}

bool d3d12_begin_frame() {
    state->frame_num = state->swapchain->GetCurrentBackBufferIndex();

    ID3D12CommandAllocator* ca = state->graphics_command_allocator[state->frame_num].Get();
    ID3D12GraphicsCommandList* cl = state->graphics_command_lists[state->frame_num].Get();
    ID3D12CommandQueue* cq = state->graphics_queue.Get();
    ID3D12Fence* fence = state->fences[state->frame_num].Get();
    ID3D12RootSignature* rs = state->naked_root_signature.Get();
    ID3D12PipelineState* pso = state->naked_graphics_pipeline.Get();
    ID3D12Resource* back_buffer = state->back_buffers[state->frame_num].Get();
    ID3D12Resource* depth_buffer = state->depth_buffer.Get();

    wait_for_fence(fence, state->fence_values[state->frame_num]);

    d3d12_res(ca->Reset());
    d3d12_res(cl->Reset(ca, nullptr));

    D3D12_VIEWPORT vp;
    D3D12_RECT scissor;
    get_viewport_and_scissor(&vp, &scissor);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = state->render_target_view_heap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += state->frame_num * state->render_target_view_heap_step_size;
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = state->depth_stencil_view_heap->GetCPUDescriptorHandleForHeapStart();
    d3d12_infocheck(cl->OMSetRenderTargets(1, &rtv, TRUE, &dsv));
    d3d12_infocheck(cl->SetGraphicsRootSignature(rs));
    d3d12_infocheck(cl->SetPipelineState(pso));

    resource_transition(cl, back_buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    resource_transition(cl, depth_buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    float clear_color[] = { 0.8f, 0.3f, 0.8f, 1.0f };
    d3d12_infocheck(cl->ClearRenderTargetView(rtv, clear_color, 1, &scissor));
    d3d12_infocheck(cl->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 1, &scissor));

    d3d12_infocheck(cl->RSSetViewports(1, &vp));
    d3d12_infocheck(cl->RSSetScissorRects(1, &scissor));

    resource_transition(cl, back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    resource_transition(cl, depth_buffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);

    d3d12_res(cl->Close());

    d3d12_infocheck(cq->ExecuteCommandLists(1, (ID3D12CommandList**)&cl));
    signal_fence(cq, fence, &state->fence_values[state->frame_num]);

    d3d12_res(state->swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING));

    return true;
}

bool d3d12_end_frame() {
    return true;
}

void* d3d12_create_texture(void* state, uint32_t width, uint32_t height) {
    return nullptr;
}
/****************************************** INTERNAL FUNCTIONS ********************************************* */
void dxgi_message_func(
    DXGI_INFO_QUEUE_MESSAGE_SEVERITY Severity,
    LPCSTR pDescription
) {
    switch (Severity) {
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
        Logger::fatal("%s", pDescription);
        break;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
        Logger::fatal("%s", pDescription);
        break;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
        Logger::warning("%s", pDescription);
        break;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
        Logger::debug("%s", pDescription);
        break;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
        Logger::info("%s", pDescription);
        break;
    default:
        Logger::info("%s", pDescription);
        break;
    }
}

bool poll_for_d3d12_messages() {
    if (state->info_queue == nullptr) return false;
    uint64_t num_messages = state->info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    if (!num_messages) return false;
    
    // i don't want to allocate a new message for each iteration.
    static constexpr size_t message_size = 1024 * 1024;
    DXGI_INFO_QUEUE_MESSAGE* message = (DXGI_INFO_QUEUE_MESSAGE*)malloc(message_size);

    for (uint64_t i = 0; i < num_messages; i++) {
        memset(message, 0, message_size);

        size_t message_len = 0;
        state->info_queue->GetMessageW(DXGI_DEBUG_ALL, i, nullptr, &message_len);
        state->info_queue->GetMessageW(DXGI_DEBUG_ALL, i, message, &message_len);

        dxgi_message_func(message->Severity, message->pDescription);
    }

    state->info_queue->ClearStoredMessages(DXGI_DEBUG_ALL);

    free(message);

    return true;
}

bool create_dxgi_factory() {
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)&state->factory))) {
        return false;
    }
    return true;
}

bool create_d3d12_device() {
    IDXGIAdapter1* adapter;
    size_t video_memory = 0;
    for (uint32_t i = 0; state->factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        Logger::debug("Found adapter: %ws", desc.Description);

        if (desc.DedicatedVideoMemory > video_memory) {
            state->adapter = adapter;
        }
    }

    if (state->adapter == nullptr) {
        return false;
    }

    d3d12_res(D3D12CreateDevice(state->adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&state->device)));

    if (state->has_debug_layer_enabled) {
        HMODULE dxgi_debug_dll = GetModuleHandleA("Dxgidebug.dll");
        typedef void(*PFN_DXGIGetDebugInterface)(const IID& riid, void** ppDebug);
        PFN_DXGIGetDebugInterface func = (PFN_DXGIGetDebugInterface)
            GetProcAddress(dxgi_debug_dll, "DXGIGetDebugInterface");

        if (func) {
            func(IID_PPV_ARGS(&state->info_queue));
        }

        FreeModule(dxgi_debug_dll);

        DXGI_INFO_QUEUE_MESSAGE_CATEGORY categories[] = {
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER,
            DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS
        };

        DXGI_INFO_QUEUE_MESSAGE_SEVERITY severities[] = {
            DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION,
            DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,
            DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING,
            /*DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO*/
        };

        DXGI_INFO_QUEUE_FILTER filter{};
        filter.AllowList.NumCategories = _countof(categories);
        filter.AllowList.pCategoryList = categories;
        filter.AllowList.NumSeverities = _countof(severities);
        filter.AllowList.pSeverityList = severities;

        d3d12_res(state->info_queue->PushStorageFilter(DXGI_DEBUG_ALL, &filter));
    }

    return true;
}

bool enable_debug_layer() {
#ifdef DEBUG
    ID3D12Debug* debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        debugController->Release();
        state->has_debug_layer_enabled = true;
        return true;
    } else {
        return false;
    }
#endif // DEBUG
    return false;
}

bool create_device_queues() {
    D3D12_COMMAND_QUEUE_DESC desc;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;
    d3d12_res(state->device->CreateCommandQueue(&desc, IID_PPV_ARGS(&state->graphics_queue)));

    return true;
}

bool create_swapchain(uint32_t width, uint32_t height) {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(state->window, &info);
    HWND hWnd = info.info.win.window;

    state->back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = state->back_buffer_format;
    desc.Stereo = false;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_NONE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> sc;

    d3d12_res(state->factory->CreateSwapChainForHwnd(
        state->graphics_queue.Get(),
        hWnd,
        &desc,
        nullptr,
        nullptr,
        &sc
    ));

    sc.As(&state->swapchain);
    state->num_frames = desc.BufferCount;

    return true;
}

bool get_swapchain_back_buffers() {
    state->back_buffers.resize(state->num_frames);
    for (uint32_t i = 0; i < state->num_frames; i++) {
        d3d12_res(state->swapchain->GetBuffer(i, IID_PPV_ARGS(&state->back_buffers[i])));
    }
    return true;
}

bool get_descriptor_heap_increment_sizes() {
    state->constant_buffer_view_heap_step_size = state->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    state->sampler_heap_step_size = state->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    state->depth_stencil_view_heap_step_size = state->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    state->render_target_view_heap_step_size = state->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    return true;
}

bool create_descriptor_heap(ID3D12DescriptorHeap** out_heap, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptor) {
    bool is_shader_visible = false;

    if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER || type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
        is_shader_visible = true;
    }

    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = type;
    desc.NumDescriptors = num_descriptor;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    d3d12_res(state->device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(out_heap)));

    return true;
}

bool create_render_target_views() {
    for (uint32_t i = 0; i < state->num_frames; i++) {
        D3D12_CPU_DESCRIPTOR_HANDLE heap_pointer = state->render_target_view_heap->GetCPUDescriptorHandleForHeapStart();
        heap_pointer.ptr += i * state->render_target_view_heap_step_size;
        state->device->CreateRenderTargetView(state->back_buffers[i].Get(), nullptr, heap_pointer);
    }
    return true;
}

bool create_fence(ID3D12Fence** out_fence) {
    d3d12_res(state->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(out_fence)));
    return true;
}

bool create_swapchain_fences() {
    state->fences.resize(state->num_frames);
    state->fence_values.resize(state->num_frames);
    for (uint32_t i = 0; i < state->num_frames; i++) {
        if (!create_fence(&state->fences[i])) {
            return false;
        }
    }

    return true;
}

bool create_depth_buffer(uint32_t width, uint32_t height) {
    state->depth_stencil_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    D3D12_HEAP_PROPERTIES heap_properties;
    heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = state->mip_levels;
    desc.Format = state->depth_stencil_format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clear_value{};
    clear_value.DepthStencil.Depth = 1.0f;
    clear_value.DepthStencil.Stencil = 0;
    clear_value.Format = state->depth_stencil_format;

    d3d12_res(state->device->CreateCommittedResource(
        &heap_properties, 
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        &clear_value,
        IID_PPV_ARGS(&state->depth_buffer)
    ));

    return true;
}

bool create_depth_view() {
    D3D12_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = state->depth_stencil_format;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_DSV_FLAG_NONE;
    desc.Texture2D.MipSlice = state->mip_levels;

    state->device->CreateDepthStencilView(state->depth_buffer.Get(), &desc, state->depth_stencil_view_heap->GetCPUDescriptorHandleForHeapStart());
    return true;
}

bool create_command_allocator(ID3D12CommandAllocator** out_allocator, D3D12_COMMAND_LIST_TYPE type) {
    d3d12_res(state->device->CreateCommandAllocator(type, IID_PPV_ARGS(out_allocator)));
    return true;
}

bool create_command_list(void** out_command_list, ID3D12CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type) {
    d3d12_res(state->device->CreateCommandList(0, type, allocator, nullptr, __uuidof(ID3D12GraphicsCommandList), out_command_list));
    static_cast<ID3D12GraphicsCommandList*>(*out_command_list)->Close();
    return true;
}

bool create_graphics_command_allocator_and_lists() {
    state->graphics_command_lists.resize(state->num_frames);
    state->graphics_command_allocator.resize(state->num_frames);
    for (uint32_t i = 0; i < state->num_frames; i++) {
        if (create_command_allocator(&state->graphics_command_allocator[i], D3D12_COMMAND_LIST_TYPE_DIRECT)) {
            if (!create_command_list(&state->graphics_command_lists[i], state->graphics_command_allocator[i].Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)) {
                return false;
            }
        } else {
            false;
        }
    }
    return true;
}

bool create_naked_pipeline_state_object() {
    {
        D3D12_ROOT_SIGNATURE_DESC desc{};
        ComPtr<ID3DBlob> root_blob;
        ComPtr<ID3DBlob> error;

        D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &root_blob, &error);
     
        if (error.Get() != nullptr) {
            Logger::fatal("%s", error->GetBufferPointer());
            return false;
        }

        state->device->CreateRootSignature(0, root_blob->GetBufferPointer(), root_blob->GetBufferSize(), IID_PPV_ARGS(&state->naked_root_signature));
    }

    ComPtr<ID3DBlob> vs;
    _temp_get_shader(vs);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
    desc.pRootSignature = state->naked_root_signature.Get();
    desc.VS.pShaderBytecode = vs->GetBufferPointer();
    desc.VS.BytecodeLength = vs->GetBufferSize();
    desc.PS = {};
    desc.DS = {};
    desc.HS = {};
    desc.GS = {};
    desc.StreamOutput = {};
    desc.BlendState = {};
    desc.SampleMask = UINT32_MAX;
    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    desc.RasterizerState.FrontCounterClockwise = false;
    desc.RasterizerState.DepthBias = 0;
    desc.RasterizerState.DepthBiasClamp = 0.0f;
    desc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    desc.RasterizerState.DepthClipEnable = true;
    desc.RasterizerState.MultisampleEnable = false;
    desc.RasterizerState.AntialiasedLineEnable = false;
    desc.RasterizerState.ForcedSampleCount = 0;
    desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    desc.DepthStencilState.DepthEnable = true;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    desc.DepthStencilState.StencilEnable = false;
    desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.InputLayout.pInputElementDescs = nullptr;
    desc.InputLayout.NumElements = 0;
    desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    memset(desc.RTVFormats, 0, sizeof(desc.RTVFormats));
    desc.RTVFormats[0] = state->back_buffer_format;
    desc.DSVFormat = state->depth_stencil_format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.NodeMask = 0;
    desc.CachedPSO.pCachedBlob = nullptr;
    desc.CachedPSO.CachedBlobSizeInBytes = 0;
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    d3d12_res(state->device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&state->naked_graphics_pipeline)));
    return true;
}

bool _temp_get_shader(ComPtr<ID3DBlob>& blob) {
    d3d12_res(D3DReadFileToBlob(L".\\naked_vert.cso", &blob));
    return true;
}

bool get_viewport_and_scissor(D3D12_VIEWPORT* out_viewport, D3D12_RECT* out_scissor) {
    out_viewport->TopLeftX = 0.0f;
    out_viewport->TopLeftY = 0.0f;
    out_viewport->Width = (float)state->width;
    out_viewport->Height = (float)state->height;
    out_viewport->MinDepth = 0.0f;
    out_viewport->MaxDepth = 1.0f;

    out_scissor->left = 0;
    out_scissor->top = 0;
    out_scissor->right = (long)state->width;
    out_scissor->bottom = (long)state->height;
    return true;
}

bool resource_transition(ID3D12GraphicsCommandList* command_list, ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) {
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = state_before;
    barrier.Transition.StateAfter = state_after;

    d3d12_infocheck(command_list->ResourceBarrier(1, &barrier));

    return true;
}

bool signal_fence(ID3D12CommandQueue* queue, ID3D12Fence* fence, uint64_t* value) {
    d3d12_res(queue->Signal(fence, ++(*value)));
    return true;
}

bool wait_for_fence(ID3D12Fence* fence, uint64_t value_to_wait) {
    uint64_t fence_value = fence->GetCompletedValue();

    if (fence_value < value_to_wait) {
        HANDLE event_handle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        d3d12_res(fence->SetEventOnCompletion(value_to_wait, event_handle));

        WaitForSingleObject(event_handle, DWORD_MAX);
        CloseHandle(event_handle);
    }

    return true;
}

bool wait_device_idle() {
    for (uint32_t i = 0; i < state->num_frames; i++) {
        wait_for_fence(state->fences[i].Get(), state->fence_values[i]);
        signal_fence(state->graphics_queue.Get(), state->fences[i].Get(), &state->fence_values[i]);
        wait_for_fence(state->fences[i].Get(), state->fence_values[i]);
    }
    return true;
}

}

#endif