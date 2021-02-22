#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "/common.glsl"

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBiTangent;

layout(set = 0, binding = 1) uniform sampler2D tDiffuse;
layout(set = 0, binding = 2) uniform sampler2D tSpecular;
layout(set = 0, binding = 3) uniform sampler2D tNormal;

layout(location = 0) out vec4 fragColor;

layout(set = 2, binding = 1) uniform WorldObject 
{
    vec3 uCameraPosition;
    Light lights[4];
    float lightCount;
} woj;

vec3 NormalMap()
{
    vec3 nNormal = texture(tNormal, fragTexCoord.xy).rgb;
    if(nNormal.z == 0.0f || fragTexCoord.x > 1.0f || fragTexCoord.x < 0.0f || fragTexCoord.y > 1.0f || fragTexCoord.y < 0.0f) return normalize(fragNormal);
    vec3 normal = normalize(fragNormal);
    vec3 tang = normalize(fragTangent);
    vec3 bTang = normalize(fragBiTangent);
    tang = normalize(tang - dot(tang, normal) * normal);
    nNormal = 2.0f * nNormal - vec3(1.0f);
    mat3 TBN = mat3(tang, bTang, normal);
    return normalize(TBN * nNormal);
}

void main()
{
    vec3 lNormal = NormalMap();
    vec3 fragDiffuse = texture(tDiffuse, fragTexCoord).rgb;
    vec4 fragSpecular = texture(tSpecular, fragTexCoord);
    vec3 directLighting = CalcDirectLighting(woj.lights[0], fragPosition, lNormal, fragDiffuse, fragSpecular, woj.uCameraPosition);
    fragColor = vec4(directLighting, 1.0f);
}