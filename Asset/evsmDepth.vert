#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBiTangent;
layout(location = 4) in vec2 inTexCoord;

layout(set = 2, binding = 0) uniform UniformBufferObject 
{
    mat4 lightView;
    mat4 lightProj;
} ubo;

layout(set = 1, binding = 0) uniform MeshData
{
    mat4 modelTransform;
} md;

layout(location = 0) out vec4 fragPosition;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
    vec4 vertexPos = vec4(inPosition, 1.0f);
    fragPosition = ubo.lightProj * ubo.lightView * md.modelTransform * vertexPos;
    fragTexCoord = inTexCoord;
    gl_Position = fragPosition;
}