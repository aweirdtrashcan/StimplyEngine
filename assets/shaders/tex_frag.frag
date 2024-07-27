#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 tex_coord;

layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

void main() {
    color = texture(tex_sampler, tex_coord);
}