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
layout(set = 0, binding = 4) uniform sampler2D tShadow;

layout(location = 0) out vec4 fragColor;

layout(set = 2, binding = 1) uniform WorldObject 
{
    vec3 uCameraPosition;
    Light lights[4];
    float lightCount;
} woj;

layout(set = 2, binding = 2) uniform ShadowMapBufferObject
{
    float expCx;
    float expCy;
} smo;

layout(set = 2, binding = 3) uniform UniformBufferObject 
{
    mat4 lightView;
    mat4 lightProj;
} ubo;

float Chebyshev(vec2 moments, float mean, float minVariance)
{
    if(mean <= moments.x)
    {
        return 1.0f;
    }
    else
    {
        float variance = moments.y - (moments.x * moments.x);
        variance = max(variance, minVariance);
        float d = mean - moments.x;
        float lit = variance / (variance + (d * d));
        if(lit < 0.5f) return 0.0f;
        return (lit - 0.5f) / 0.5f;
    }
}

float Visibility(vec3 position)
{
    vec4 lsPos = ubo.lightProj * ubo.lightView * vec4(position, 1.0f);
    if(lsPos.w == 0.0f) return 1.0f;
    lsPos /= lsPos.w;
    lsPos.xy = 0.5f * lsPos.xy + vec2(0.5f);
    vec4 moments = texture(tShadow, lsPos.xy);
    vec2 wDepth = WarpDepth(lsPos.z - 0.0001f, smo.expCx, smo.expCy);
    vec2 exponents = vec2(smo.expCx, smo.expCy);
    vec2 depthScale = 0.0001f * exponents * wDepth;
    vec2 minVariance = depthScale * depthScale;
    float positive = Chebyshev(moments.xz, wDepth.x, minVariance.x);
    float negative = Chebyshev(moments.yw, wDepth.y, minVariance.y);
    return negative;
}

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

vec3 CalcDirectLighting(Light cLight, vec3 position, vec3 normal, vec3 mDiffuse, vec4 mSpecular, vec3 uCameraPosition)
{
    vec3 ambient = vec3(0.5f, 0.5f, 0.5f);
    vec3 L = cLight.direction;
    vec3 V = (normalize(uCameraPosition - position)).xyz;
    vec3 H = normalize(L + V);
    vec3 N = normal;
    float dotNL = max(dot(N, L), 0.0f);
    float dotNH = max(dot(N, H), 0.0f);
    float dotLH = max(dot(L, H), 0.0f);
    float spec = exp2(11.0f * mSpecular.a + 1.0f);
    vec3 fresnel = mSpecular.rgb + (1.0f - mSpecular.rgb) * pow(1.0f - dotLH, 5.0f);
    float blinnPhong = pow(dotNH, spec);
    blinnPhong *= spec * 0.0397f + 0.3183f;
    vec3 specular = mSpecular.rgb * cLight.specular * blinnPhong * fresnel;
    vec3 diffuse = mDiffuse.rgb * cLight.diffuse;
    vec3 amb = ambient * mDiffuse.rgb;
    return diffuse * Visibility(fragPosition) + amb;
}

void main()
{
    vec3 lNormal = NormalMap();
    vec3 fragDiffuse = texture(tDiffuse, fragTexCoord).rgb;
    vec4 fragSpecular = texture(tSpecular, fragTexCoord);
    vec3 directLighting = CalcDirectLighting(woj.lights[0], fragPosition, lNormal, fragDiffuse, fragSpecular, woj.uCameraPosition);
    fragColor = vec4(directLighting, 1.0f);
}