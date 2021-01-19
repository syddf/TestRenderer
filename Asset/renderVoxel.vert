#version 430
#extension GL_ARB_shader_image_load_store : require

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    float dimension;
} ubo;

void main()
{
    highp int uDimension = int(ubo.dimension);
    vec3 position = vec3(gl_VertexIndex % uDimension, (gl_VertexIndex / uDimension) % uDimension, gl_VertexIndex / (uDimension * uDimension));
    gl_Position = vec4(position, 0.0f);
}