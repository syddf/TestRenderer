struct Light
{
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    vec3 direction;
};

vec4 convRGBA8ToVec4(uint val)
{
    return vec4(float((val & 0x000000FF)), 
    float((val & 0x0000FF00) >> 8U), 
    float((val & 0x00FF0000) >> 16U), 
    float((val & 0xFF000000) >> 24U));
}

uint convVec4ToRGBA8(vec4 val)
{
    return (uint(val.w) & 0x000000FF) << 24U | 
    (uint(val.z) & 0x000000FF) << 16U | 
    (uint(val.y) & 0x000000FF) << 8U | 
    (uint(val.x) & 0x000000FF);
}

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}

vec3 BRDF(Light light, vec3 normal, vec3 albedo)
{
    return albedo * light.diffuse * max(dot(light.direction, normal), 0);
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
    return (diffuse + specular) * dotNL + amb;
}