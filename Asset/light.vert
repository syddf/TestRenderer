#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding = 0) uniform MeshData
{
    mat4 modelTransform;
    mat4 normalTransform;
} md;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBiTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBiTangent;

void main()
{
    gl_Position = ubo.proj * ubo.view * md.modelTransform * vec4(inPosition, 1.0);
    fragPosition = (md.modelTransform * vec4(inPosition, 1.0)).xyz;
    fragNormal = (md.normalTransform * vec4(inNormal, 0.0)).xyz;
    fragTangent = (md.normalTransform * vec4(inTangent, 0.0)).xyz;
    fragBiTangent = (md.normalTransform * vec4(inBiTangent, 0.0)).xyz;
    fragTexCoord = inTexCoord;
    fragTexCoord.y = 1.0f - fragTexCoord.y;
}