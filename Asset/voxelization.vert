#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

layout(set = 1, binding = 0) uniform MeshData
{
    mat4 modelTransform;
    mat4 normalTransform;
} md;

void main()
{
    gl_Position = md.modelTransform * vec4(inPosition, 1.0f);
    fragNormal = (md.normalTransform * vec4(inNormal, 0.0f)).xyz;
    fragTexCoord = inTexCoord;
}