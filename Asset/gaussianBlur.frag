#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform MatParams
{
    float padding;
}moj;

layout(set = 1, binding = 0) uniform ObjParams
{
   float padding;
}ooj;

layout(set = 2, binding = 0) uniform sampler2D blurTexture;

layout(push_constant) uniform pushBlock 
{
    float blurDirectionX;
    float blurDirectionY;
} blurConstant;

vec4 blur9(sampler2D image, vec2 uv, vec2 direction) 
{
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.3846153846) * direction;
	vec2 off2 = vec2(3.2307692308) * direction;
	color += texture(image, uv) * 0.2270270270;
	color += texture(image, uv + off1) * 0.3162162162;
	color += texture(image, uv - off1) * 0.3162162162;
	color += texture(image, uv + off2) * 0.0702702703;
	color += texture(image, uv - off2) * 0.0702702703;
	return color;
}

void main()
{
	vec2 blurDirection = vec2(blurConstant.blurDirectionX, blurConstant.blurDirectionY);
	outColor = blur9(blurTexture, texCoord, blurDirection);
}
