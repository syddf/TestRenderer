#version 430
#extension GL_ARB_separate_shader_objects : enable

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

layout(set = 0, binding = 1) uniform Matrix
{
    mat4 view;
    mat4 proj;
} matrices;

layout(set = 0, binding = 2) uniform VoxelParams
{
    vec3 worldMinPoint;
    float voxelSize;
} params;

layout(location = 0)out vec4 voxelColor;

struct ViewFrustum
{
     vec4 planes[6];
};

bool InFrustum(vec4 viewPos, ViewFrustum frustum)
{
    bool res = true;
    for( int i = 0 ; i < 6 ; i ++ ) 
    {
        if( ( dot( viewPos.xyz , frustum.planes[i].xyz) + frustum.planes[i].w )  < -dot(vec3(params.voxelSize * 0.5f), abs(frustum.planes[i].xyz)) ) 
        {
	res = false;
	break;
        }
    }
    return res;
}

void main()
{
    const vec4 cubeVertices[8] = vec4[8] 
    (
        vec4( 0.5f,  0.5f,  0.5f, 0.0f),
        vec4( 0.5f,  0.5f, -0.5f, 0.0f),
        vec4( 0.5f, -0.5f,  0.5f, 0.0f),
        vec4( 0.5f, -0.5f, -0.5f, 0.0f),
        vec4(-0.5f,  0.5f,  0.5f, 0.0f),
        vec4(-0.5f,  0.5f, -0.5f, 0.0f),
        vec4(-0.5f, -0.5f,  0.5f, 0.0f),
        vec4(-0.5f, -0.5f, -0.5f, 0.0f)
    );

    const int cubeIndices[24]  = int[24] 
    (
        0, 2, 1, 3,
        6, 4, 7, 5,
        5, 4, 1, 0,
        6, 7, 2, 3,
        4, 6, 0, 2,
        1, 3, 5, 7
    );
    const vec4 color[4] = vec4[4] 
    (
        vec4( 1.0f,  0.0f,  0.0f, 1.0f),
        vec4( 0.0f,  1.0f,  0.0f, 1.0f),
        vec4( 0.0f,  0.0f,  1.0f, 1.0f),
        vec4( 1.0f,  1.0f,  1.0f, 1.0f)
    );

    vec4 col0 = vec4(matrices.proj[0][0], matrices.proj[1][0], matrices.proj[2][0], matrices.proj[3][0]);
    vec4 col1 = vec4(matrices.proj[0][1], matrices.proj[1][1], matrices.proj[2][1], matrices.proj[3][1]);
    vec4 col2 = vec4(matrices.proj[0][2], matrices.proj[1][2], matrices.proj[2][2], matrices.proj[3][2]);
    vec4 col3 = vec4(matrices.proj[0][3], matrices.proj[1][3], matrices.proj[2][3], matrices.proj[3][3]);

    ViewFrustum frustum;
    frustum.planes[0] = col3 + col0;
    frustum.planes[1] = col3 - col0;
    frustum.planes[2] = col3 + col1;
    frustum.planes[3] = col3 - col1;
    frustum.planes[4] = col3 + col2;
    frustum.planes[5] = col3 - col2;
    vec4 worldPos = gl_in[0].gl_Position * params.voxelSize + vec4(params.worldMinPoint, 1.0f);
    vec4 viewPos = matrices.view * worldPos;
    if(!InFrustum(viewPos, frustum))
    {
	return;
    }
    
    vec4 projectedVertices[8];

    

    for(int i = 0; i < 8; i++)
    {
        vec4 vertex = (gl_in[0].gl_Position + cubeVertices[i]) * params.voxelSize + vec4(params.worldMinPoint, 1.0f);
        projectedVertices[i] = matrices.proj * matrices.view * vertex;
    }
    for(int face = 0; face < 6; ++face)
    {
        for(int vertex = 0; vertex < 4; ++vertex)
        {
            gl_Position = projectedVertices[cubeIndices[face * 4 + vertex]];
            voxelColor = color[vertex];
            EmitVertex();
        }

        EndPrimitive();
    }
}