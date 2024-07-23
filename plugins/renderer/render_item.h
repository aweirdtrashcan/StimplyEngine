#pragma once

#include <containers/list.h>

#include "defines.h"
#include "renderer/renderer_types.h"
#include "vulkan/vulkan_defines.h"

struct shader_data {
    gpu_buffer buffer;
    HANDLE buffer_descriptor;
};

struct shader_object {
    PipelineStage pipeline_stage;
    shader_data data;
};

struct render_item {
    gpu_buffer vertex_buffer;
    gpu_buffer index_buffer;
    uint32_t index_count;
    uint32_t vertices_count;
    list<shader_object> shader_objects;
};