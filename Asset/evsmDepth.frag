#version 450

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler2D tDiffuse;

layout(set = 2, binding = 1) uniform ShadowMapBufferObject
{
    float expCx;
    float expCy;
} smo;

vec2 Depth(float d)
{
    d = 2.0f * d - 1.0f;
    float pos = exp(smo.expCx * d);
    float neg = -exp(-smo.expCy * d);
    return vec2(pos, neg);
}

vec4 DepthToEVSM(float d)
{
    vec2 moment1 = Depth(d);
    vec2 moment2 = moment1 * moment1;
    return vec4(moment1, moment2);
}

void main()
{
     vec4 diffuseColor = texture(tDiffuse, fragTexCoord);
     fragColor = DepthToEVSM(gl_FragCoord.z);
}