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

layout(set = 2, binding = 4) uniform sampler3D voxelRadiance_IN;

layout(set = 2, binding = 5) uniform sampler3D voxelMipMapAlbedo_IN[6];

layout(set = 2, binding = 6) uniform VoxelParams
{
    vec3 worldMinPoint;
    float voxelSize;
    float voxelDimension;
} params;

const float PI = 3.14159265f;

vec3 WorldToVoxel(vec3 position)
{
    float voxelScale = 1.0f / (params.voxelSize * params.voxelDimension);
    vec3 voxelPos = position - params.worldMinPoint;
    return voxelPos * voxelScale;
}

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
    vec3 ambient = vec3(0.01f, 0.01f, 0.01f);
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

vec4 AnistropicSample(vec3 coord, vec3 weight, uvec3 face, float lod)
{
    float anisoLevel = max(lod - 1.0f, 0.0f);
    vec4 anisoSample = weight.x * textureLod(voxelMipMapAlbedo_IN[face.x], coord, anisoLevel)
                     + weight.y * textureLod(voxelMipMapAlbedo_IN[face.y], coord, anisoLevel)
                     + weight.z * textureLod(voxelMipMapAlbedo_IN[face.z], coord, anisoLevel);
    if(lod < 1.0f)
    {
        vec4 baseColor = texture(voxelRadiance_IN, coord);
        anisoSample = mix(baseColor, anisoSample, clamp(lod, 0.0f, 1.0f));
    }

    return anisoSample;                    
}

bool IntersectRayWithWorldAABB(vec3 ro, vec3 rd)
{
    float enter = 0.0; 
    float leave = 0.0;
    float worldSize = params.voxelSize * params.voxelDimension;
    vec3 worldMaxPoint = params.worldMinPoint + vec3(worldSize);
    vec3 tempMin = (params.worldMinPoint - ro) / rd; 
    vec3 tempMax = (worldMaxPoint - ro) / rd;
    vec3 v3Max = max (tempMax, tempMin);
    vec3 v3Min = min (tempMax, tempMin);
    leave = min (v3Max.x, min (v3Max.y, v3Max.z));
    enter = max (max (v3Min.x, 0.0), max (v3Min.y, v3Min.z));    
    return leave > enter;
}

vec4 TraceCone(vec3 position, vec3 normal, vec3 direction, float aperture)
{
    uvec3 visibleFace;
    visibleFace.x = (direction.x < 0.0) ? 0 : 1;
    visibleFace.y = (direction.y < 0.0) ? 2 : 3;
    visibleFace.z = (direction.z < 0.0) ? 4 : 5;
    float voxelScale = params.voxelSize * params.voxelDimension;
    float voxelWorldSize = params.voxelSize * 2;
    vec3 weight = direction * direction;
    float dst = voxelWorldSize;
    vec3 startPosition = position + normal * dst;
    vec4 coneSample = vec4(0.0f);
    float occlusion = 0.0f;
    float maxDistance = voxelScale;
    float falloff = 0.5f * 725.0f / voxelScale;

    while(coneSample.a < 1.0f && dst <= maxDistance)
    {
        vec3 conePosition = startPosition + direction * dst;
        float diameter = 2.0f * aperture * dst;
        float mipLevel = log2(diameter / voxelWorldSize);
        vec3 coord = WorldToVoxel(conePosition);
        vec4 anisoSample = AnistropicSample(coord, weight, visibleFace, mipLevel);
        coneSample += (1.0f - coneSample.a) * anisoSample;
        if(occlusion < 1.0)
        {
            occlusion += ((1.0f - occlusion) * anisoSample.a) / (1.0f + falloff * diameter);
        }
        dst += diameter * 0.5f;
    }

    return vec4(coneSample.rgb, occlusion);
}

vec4 CalculateIndirectLighting(vec3 position, vec3 normal, vec3 albedo, vec4 specular)
{
    normal = normalize(normal);
    const vec3 diffuseConeDirections[] =
    {
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 0.5f, 0.866025f),
        vec3(0.823639f, 0.5f, 0.267617f),
        vec3(0.509037f, 0.5f, 0.7006629f),
        vec3(-0.50937f, 0.5f, 0.7006629f),
        vec3(-0.823639f, 0.5f, 0.267617f)
    };

    const float diffuseConeWeights[] =
    {
        PI / 4.0f,
        3.0f * PI / 20.0f,
        3.0f * PI / 20.0f,
        3.0f * PI / 20.0f,
        3.0f * PI / 20.0f,
        3.0f * PI / 20.0f,
     };

    vec4 diffuseTrace = vec4(0.0f);
    vec3 coneDirection = vec3(0.0f);

    if(any(greaterThan(albedo, diffuseTrace.rgb)))
    {
        const float aperture = 0.57735f;
        vec3 guide = vec3(0.0f, 1.0f, 0.0f);
        if (abs(dot(normal,guide)) >= 0.95)
        {
            guide = vec3(0.0f, 0.0f, 1.0f);
        }
        vec3 right = normalize(guide - dot(normal, guide) * normal);
        vec3 up = cross(right, normal);

        for(int i = 0; i < 6; i++)
        {
            coneDirection = normal;
            coneDirection += diffuseConeDirections[i].x * right + diffuseConeDirections[i].z * up;
            coneDirection = normalize(coneDirection);
            diffuseTrace += TraceCone(position, normal, coneDirection, aperture) * diffuseConeWeights[i];
        }

        diffuseTrace.rgb *= albedo;
    }

    vec3 result = diffuseTrace.rgb;

    return diffuseTrace;
}

void main()
{
    vec3 lNormal = NormalMap();
    vec3 fragDiffuse = texture(tDiffuse, fragTexCoord).rgb;
    vec4 fragSpecular = texture(tSpecular, fragTexCoord);
    vec3 directLighting = CalcDirectLighting(woj.lights[0], fragPosition, lNormal, fragDiffuse, fragSpecular, woj.uCameraPosition);
    vec4 indirectLighting = CalculateIndirectLighting(fragPosition, fragNormal, fragDiffuse, fragSpecular);

    vec3 compositeLighting = directLighting + indirectLighting.xyz * 2.0f;
    compositeLighting = compositeLighting / (compositeLighting + 1.0f);
    const float gamma = 2.2;
    compositeLighting = pow(compositeLighting, vec3(1.0 / gamma));

    fragColor = vec4(1.0f - indirectLighting.w, 1.0f - indirectLighting.w, 1.0f - indirectLighting.w , 1.0f);
}