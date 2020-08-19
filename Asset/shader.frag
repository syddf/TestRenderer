#version 450
#extension GL_ARB_separate_shader_objects : enable
struct Light
{
vec3 a;
float b[4];
};
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform Block
{
	Light l1;
	Light l2;
};

layout(binding = 3) uniform sampler2D tex;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}