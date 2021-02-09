#version 450
#extension GL_ARB_shader_image_load_store : require

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    float empty;
} ubo;

layout(set = 1, binding = 0) uniform EmptyBuffer
{
    float empty;
} eb;

layout(set = 2, binding = 0, r32ui) uniform uimage3D voxelAlbedo_IN;
layout(location = 0)out vec4 albedo;

layout(set = 2, binding = 1) uniform VoxelParams
{
    vec3 worldMinPoint;
    float voxelSize;
    float voxelDimension;
} params;

vec4 convRGBA8ToVec4(uint val)
{
    return vec4(float((val & 0x000000FF)), 
    float((val & 0x0000FF00) >> 8U), 
    float((val & 0x00FF0000) >> 16U), 
    float((val & 0xFF000000) >> 24U));
}

void main()
{
    highp int uDimension = int(params.voxelDimension);
    vec3 position = vec3(gl_VertexIndex % uDimension, (gl_VertexIndex / uDimension) % uDimension, gl_VertexIndex / (uDimension * uDimension));
    ivec3 texPos = ivec3(position);
    uint va = imageLoad(voxelAlbedo_IN, texPos).r;
    albedo = convRGBA8ToVec4(va) / 255.0f;
    gl_Position = vec4(position, 0.0f);
}