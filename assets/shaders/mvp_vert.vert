#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_tex_coord;

layout(location = 0) out vec2 out_tex_coord;

layout(set = 0, binding = 0) uniform global_uniform_buffer {
    mat4 projection;
    mat4 view;
} global_ubo;

layout(push_constant) uniform push_constants {
    mat4 model;
} push_const;

void main() {
    gl_Position = global_ubo.projection * global_ubo.view * push_const.model * vec4(in_position, 1.0);

    out_tex_coord = in_tex_coord;
}