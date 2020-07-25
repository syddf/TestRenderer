#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include "Source/Rasterize.h"
#include "Source/StaticLighting.h"
#include "Source/OutputImageHelper.h"
#include "Source/BVH.h"

int main() 
{
	/*
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::cout << extensionCount << " extensions supported\n";
    
    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

	int UpsampleFactor = 4;
	int HighResolutionSignalSizeX = 256 * 4;
	int HighResolutionSignalSizeY = 256 * 4;

	DistanceFieldRasterPolicy RasterPolicy(UpsampleFactor, HighResolutionSignalSizeX, HighResolutionSignalSizeY);
	TriangleRasterizer<DistanceFieldRasterPolicy> DistanceFieldRasterizer(RasterPolicy);
	StaticLightingVertex v0;
	StaticLightingVertex v1;
	StaticLightingVertex v2;
	Vec2 uv0;
	Vec2 uv1;
	Vec2 uv2;
	DistanceFieldRasterizer.DrawTriangle(v0, v1, v2, uv0, uv1, uv2);
    while(!glfwWindowShouldClose(window)) 
	{
        glfwPollEvents();
    } 
    glfwDestroyWindow(window);
    glfwTerminate();    
	*/
	std::string path = "./../Asset/Dst/Cornell.data";
	ImportAsset* scene= new ImportSceneData();
	scene->Deserialize(path);

	size_t MeshCount = static_cast<ImportSceneData*>(scene)->MeshData.GetMeshCount();
	std::vector<StaticLightingMesh> LightingMeshVec(MeshCount);

	for (size_t i = 0; i < MeshCount; i ++)
	{
		LightingMeshVec[i].InitFromImportMesh(*static_cast<ImportSceneData*>(scene), i);
	}

	int UpsampleFactor = 4;
	int HighResolutionSignalSizeX = 256 * 4;
	int HighResolutionSignalSizeY = 256 * 4;

	DistanceFieldRasterPolicy RasterPolicy(UpsampleFactor, HighResolutionSignalSizeX, HighResolutionSignalSizeY);
	TriangleRasterizer<DistanceFieldRasterPolicy> DistanceFieldRasterizer(RasterPolicy);

	StaticLightingMesh& mesh = LightingMeshVec[0];
	for (int i = 0; i < mesh.Indices.size();)
	{
		StaticLightingVertex v0 = mesh.Vertices[mesh.Indices[i]];
		StaticLightingVertex v1 = mesh.Vertices[mesh.Indices[i + 1]];
		StaticLightingVertex v2 = mesh.Vertices[mesh.Indices[i + 2]];
		Vec2 uv0 = mesh.Vertices[mesh.Indices[i]].TexCoord1;
		Vec2 uv1 = mesh.Vertices[mesh.Indices[i + 1]].TexCoord1;
		Vec2 uv2 = mesh.Vertices[mesh.Indices[i + 2]].TexCoord1;
		DistanceFieldRasterizer.DrawTriangle(v0, v1, v2, uv0, uv1, uv2);
		i += 3;
	}

	std::vector<UInt8> Pixels;
	Pixels.reserve(HighResolutionSignalSizeX * HighResolutionSignalSizeY * 4);
	for (int i = 0; i < DistanceFieldRasterizer.Data.size(); i ++)
	{
		Pixels.push_back(std::abs(DistanceFieldRasterizer.Data[i].x * 255));
		Pixels.push_back(std::abs(DistanceFieldRasterizer.Data[i].y * 255));
		Pixels.push_back(std::abs(DistanceFieldRasterizer.Data[i].z * 255));
		Pixels.push_back(255);
	}
	OutputImageHelper::OutputPNG("tmp.png", Pixels, HighResolutionSignalSizeX, HighResolutionSignalSizeY);
    return 0;
}
