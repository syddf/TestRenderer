#version 450

#include "/common.glsl"
layout(location = 0) in vec3 inWSPosition;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec4 inTriangleAABB;

layout(set = 0, binding = 0) uniform sampler2D tDiffuse;

layout(set = 1, binding = 1) uniform ObjectParams
{
   float staticVoxelFlag;
} voxelParams;

layout(set = 2, binding = 0, r32ui) uniform uimage3D voxelAlbedo_IN;
layout(set = 2, binding = 1, r32ui) uniform uimage3D voxelNormal_IN;
layout(set = 2, binding = 2, r8) uniform image3D staticVoxelFlag_IN;

void imageAtomicRGBA8AvgAlbedo(ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;

    while((curStoredVal = imageAtomicCompSwap(voxelAlbedo_IN, coords, prevStoredVal, newVal)) 
            != prevStoredVal)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);
    }
}

void imageAtomicRGBA8AvgNormal(ivec3 coords, vec4 value)
{
   value.rgb *= 255;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;

    while((curStoredVal = imageAtomicCompSwap(voxelNormal_IN, coords, prevStoredVal, newVal)) 
            != prevStoredVal)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);
    }
}

void main()
{
    	if( inPosition.x < inTriangleAABB.x || inPosition.y < inTriangleAABB.y || 
		inPosition.x > inTriangleAABB.z || inPosition.y > inTriangleAABB.w )
	{
		discard;
	}
	ivec3 position = ivec3(inWSPosition);
	vec4 albedo = texture(tDiffuse, inTexCoord.xy);
	float opacity = albedo.a;
 	if(opacity > 0.0f)
	{
		albedo.a = 1.0f;
        		vec4 normal = vec4(EncodeNormal(normalize(inNormal)), 1.0f);
    		imageAtomicRGBA8AvgAlbedo(position, albedo);
        		imageAtomicRGBA8AvgNormal(position, normal);
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