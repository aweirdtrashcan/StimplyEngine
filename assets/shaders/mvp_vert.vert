#version 450
#extension GL_ARB_separate_shader_objects : enable

// layout(set = 0, binding = 0) uniform global_uniform_buffer {
//     mat4 projection;
//     mat4 view;
// } global_ubo;

layout(location = 0) in vec3 in_position;

void main() {
    gl_Position = vec4(in_position, 1.0);
}