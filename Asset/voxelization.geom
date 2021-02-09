#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec3 inNormal[3];
layout (location = 1) in vec2 inTexCoord[3];

layout(set = 2, binding = 4) uniform Matrix
{
    mat4 DirectionViewProjection[3];
    mat4 DirectionInverseViewProjection[3];
} matrices;

layout(set = 2, binding = 5) uniform VoxelParams
{
    vec3 worldMinPoint;
    float voxelSize;
    float voxelDimension;
} params;

layout(set = 1, binding = 0) uniform MeshData
{
    mat4 modelTransform;
    mat4 normalTransform;
} md;

layout(location = 0) out vec3 outWSPosition;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTexCoord;
layout(location = 4) out vec4 outTriangleAABB;

int CalculateAxis()
{
	vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 faceNormal = cross(p1, p2);

	float nDX = abs(faceNormal.x);
	float nDY = abs(faceNormal.y);
	float nDZ = abs(faceNormal.z);

	if( nDX > nDY && nDX > nDZ )
    	{
		return 0;
	}
	else if( nDY > nDX && nDY > nDZ  )
    	{
	    return 1;
    	}
	else
	{
	    return 2;
	}
}

vec4 AxisAlignedBoundingBox(vec4 pos[3], vec2 pixelDiagonal)
{
	vec4 aabb;
	aabb.xy = min(pos[2].xy, min(pos[1].xy, pos[0].xy));
	aabb.zw = max(pos[2].xy, max(pos[1].xy, pos[0].xy));
	aabb.xy -= pixelDiagonal;
	aabb.zw += pixelDiagonal;
	return aabb;
}

void main()
{
	int mainAxis = CalculateAxis();
	mat4 viewProj = matrices.DirectionViewProjection[mainAxis];
	mat4 inverseViewProj = matrices.DirectionInverseViewProjection[mainAxis];
	vec4 pos[3] = vec4[3]
	(
		viewProj * gl_in[0].gl_Position,
		viewProj * gl_in[1].gl_Position,
		viewProj * gl_in[2].gl_Position
	);
	vec4 trianglePlane;
	trianglePlane.xyz = cross(pos[1].xyz - pos[0].xyz, pos[2].xyz - pos[0].xyz);
	trianglePlane.xyz = normalize(trianglePlane.xyz);
	trianglePlane.w = -dot(pos[0].xyz, trianglePlane.xyz);
	vec3 texCoord[3];
	texCoord[0].xy = inTexCoord[0];
	texCoord[1].xy = inTexCoord[1];
	texCoord[2].xy = inTexCoord[2];
    	if (dot(trianglePlane.xyz, vec3(0.0, 0.0, 1.0)) < 0.0)
    	{
        	 	vec4 vertexTemp = pos[2];
        		vec3 texCoordTemp = texCoord[2];
        
        		pos[2] = pos[1];
        		texCoord[2] = texCoord[1];
    
        		pos[1] = vertexTemp;
       		texCoord[1] = texCoordTemp;
    	}

	vec2 halfPixel = vec2(1.0f / params.voxelDimension);
 	if(trianglePlane.z == 0.0f) return;
	vec3 planes[3];
                vec4 aabb = AxisAlignedBoundingBox(pos, halfPixel);
	//outTriangleAABB = AxisAlignedBoundingBox(pos, halfPixel);
	planes[0] = cross(pos[0].xyw - pos[2].xyw, pos[2].xyw);
	planes[1] = cross(pos[1].xyw - pos[0].xyw, pos[0].xyw);
	planes[2] = cross(pos[2].xyw - pos[1].xyw, pos[1].xyw);
	planes[0].z -= dot(halfPixel, abs(planes[0].xy));
	planes[1].z -= dot(halfPixel, abs(planes[1].xy));
	planes[2].z -= dot(halfPixel, abs(planes[2].xy));
	vec3 intersection[3];
	intersection[0] = cross(planes[0], planes[1]);
	intersection[1] = cross(planes[1], planes[2]);
	intersection[2] = cross(planes[2], planes[0]);
	intersection[0] /= intersection[0].z;
	intersection[1] /= intersection[1].z;
	intersection[2] /= intersection[2].z;
	float z[3];
	z[0] = -(intersection[0].x * trianglePlane.x + intersection[0].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	z[1] = -(intersection[1].x * trianglePlane.x + intersection[1].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	z[2] = -(intersection[2].x * trianglePlane.x + intersection[2].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	pos[0].xyz = vec3(intersection[0].xy, z[0]);
	pos[1].xyz = vec3(intersection[1].xy, z[1]);
	pos[2].xyz = vec3(intersection[2].xy, z[2]);

	for(int i = 0; i < 3; ++i)
	{
		vec4 voxelPos = inverseViewProj * pos[i];
		voxelPos.xyz /= voxelPos.w;
		voxelPos.xyz -= params.worldMinPoint;
		voxelPos /= (params.voxelSize * params.voxelDimension);

		gl_Position = pos[i];
		outPosition = pos[i].xyz;
		outNormal = inNormal[i];
		outTexCoord = texCoord[i];
		outTexCoord.y = 1.0f - outTexCoord.y;
		outWSPosition = voxelPos.xyz * params.voxelDimension;
		outTriangleAABB = aabb;
		EmitVertex();
	}

	EndPrimitive();
}