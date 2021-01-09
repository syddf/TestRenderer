#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform sampler2D tDiffuse;
layout(set = 0, binding = 2) uniform sampler2D tSpecular;

layout(location = 0) out vec4 fragColor;

struct Light
{
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    vec3 direction;
};

layout(set = 0, binding = 3) uniform WorldObject 
{
    vec3 uCameraPosition;
    Light lights[4];
} woj;

vec3 CalcDirectLighting(Light cLight, vec3 position, vec3 normal, vec3 mDiffuse, vec4 mSpecular)
{
    vec3 L = cLight.direction;
    vec3 V = (normalize(woj.uCameraPosition - position)).xyz;
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
    return (diffuse + specular) * dotNL;
}

void main()
{
    vec3 fragDiffuse = texture(tDiffuse, fragTexCoord).rgb;
    vec4 fragSpecular = texture(tSpecular, fragTexCoord);
    vec3 directLighting = CalcDirectLighting(woj.lights[0], fragPosition, fragNormal, fragDiffuse, fragSpecular);
    fragColor = vec4(directLighting, 1.0f);
}