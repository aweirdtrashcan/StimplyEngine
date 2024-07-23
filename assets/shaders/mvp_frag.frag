#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 color;
layout(location = 0) in vec3 in_position;

void main() {
    color = vec4(in_position.x, in_position.y, (in_position.x / 2.0) + (in_position.y / 2.0), 1.0f);
}