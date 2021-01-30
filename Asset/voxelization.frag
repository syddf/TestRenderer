#version 450

layout(location = 0) in vec3 inWSPosition;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTriangleAABB;
layout(location = 4) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform sampler2D tDiffuse;
layout(set = 0, binding = 1) uniform sampler2D opacityMap;
layout(set = 0, binding = 2) uniform sampler2D emissiveMap;

layout(set = 1, binding = 1) uniform ObjectParams
{
   float staticVoxelFlag;
} voxelParams;

layout(set = 2, binding = 0, r32ui) uniform uimage3D voxelAlbedo_IN;
layout(set = 2, binding = 1, r32ui) uniform uimage3D voxelNormal_IN;
layout(set = 2, binding = 2, r32ui) uniform uimage3D voxelEmission_IN;
layout(set = 2, binding = 3, r8) uniform image3D staticVoxelFlag_IN;

layout(location = 0) out vec4 fragColor;

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

void imageAtomicRGBA8AvgEmission(ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;
    uint numIterations = 0;

    while((curStoredVal = imageAtomicCompSwap(voxelEmission_IN, coords, prevStoredVal, newVal)) 
            != prevStoredVal
            && numIterations < 255)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);

        ++numIterations;
    }
}

void imageAtomicRGBA8AvgAlbedo(ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;
    uint numIterations = 0;

    while((curStoredVal = imageAtomicCompSwap(voxelAlbedo_IN, coords, prevStoredVal, newVal)) 
            != prevStoredVal
            && numIterations < 255)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);

        ++numIterations;
    }
}

void imageAtomicRGBA8AvgNormal(ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;
    uint numIterations = 0;

    while((curStoredVal = imageAtomicCompSwap(voxelNormal_IN, coords, prevStoredVal, newVal)) 
            != prevStoredVal
            && numIterations < 255)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);

        ++numIterations;
    }
}

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}

void main()
{
  	if( inPosition.x < inTriangleAABB.x || inPosition.y < inTriangleAABB.y || inPosition.x > inTriangleAABB.z || inPosition.y > inTriangleAABB.w )
	{
		discard;
	}

	ivec3 position = ivec3(inWSPosition);
	vec4 albedo = texture(tDiffuse, inTexCoord.xy);
	float opacity = albedo.a;
    	if(voxelParams.staticVoxelFlag > 0)
	{
		bool isStatic = imageLoad(staticVoxelFlag_IN, position).r > 0.0f;
		if(isStatic)
		{
        			opacity = 0.0f;
		}
    	}
 	if(opacity > 0.0f)
	{
		albedo.rgb = albedo.rgb;
        		albedo.rgb *= opacity;
        		albedo.a = 1.0f;
        		vec4 emissive = texture(emissiveMap, inTexCoord.xy);
       		emissive.a = 1.0f;
        		vec4 normal = vec4(EncodeNormal(normalize(inNormal)), 1.0f);
        		imageAtomicRGBA8AvgNormal(position, normal);
        		imageAtomicRGBA8AvgAlbedo(position, albedo);
        		imageAtomicRGBA8AvgEmission(position, emissive);
        		if(voxelParams.staticVoxelFlag > 0.0f)
        		{
            			imageStore(staticVoxelFlag_IN, position, vec4(1.0));
        		}
		else
		{
            			imageStore(staticVoxelFlag_IN, position, vec4(0.0));
		}
	}
}