#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include "Source/Rasterize.h"
#include "Source/StaticLighting.h"
#include "Source/OutputImageHelper.h"
#include "Source/BVH.h"
#include "Source/StaticLighting.h"

int main() 
{
	std::string path = "./../Asset/Dst/Cornell.data";
	ImportAsset* scene= new ImportSceneData();
	scene->Deserialize(path);
	size_t MeshCount = static_cast<ImportSceneData*>(scene)->MeshData.GetMeshCount();
	std::vector<StaticLightingMesh> LightingMeshVec(MeshCount);
	for (size_t i = 0; i < MeshCount; i++)
	{
		LightingMeshVec[i].InitFromImportMesh(*static_cast<ImportSceneData*>(scene), i);
	}
	StaticLightingSystem StaticLighting;
	DirectionalLight Light;
	Light.Position = Vec3(-6480025.50, 5497625.50, 5271184.50);
	Light.Direction = Vec3(1.0f, 0.0f, 0.0f);
	StaticLighting.RunStaticLighting(LightingMeshVec, 512, 512, Light);

	/*

	size_t MeshCount = static_cast<ImportSceneData*>(scene)->MeshData.GetMeshCount();
	std::vector<StaticLightingMesh> LightingMeshVec(MeshCount);

	for (size_t i = 0; i < MeshCount; i ++)
	{
		LightingMeshVec[i].InitFromImportMesh(*static_cast<ImportSceneData*>(scene), i);
	}

	int UpsampleFactor = 4;
	int HighResolutionSignalSizeX = 256 * 4;
	int HighResolutionSignalSizeY = 256 * 4;

	StaticLightingMesh& mesh = LightingMeshVec[0];
	std::vector<Triangle> Triangles;
	mesh.GetTriangleVec(Triangles);
	BVHTree BVHTree;
	BVHTree.BuildBVHTree(Triangles);

	Point center = BVHTree.NodeVec[0].Bounding.GetCentroid();

	float ScreenWorldWidth = 1200.0f;
	float ScreenWorldHeight = 1200.0f;
	float ScreenNearZ = 900.0f;
	center.x = -2600;
	center.y = 230;
	center.z = 460;

	std::vector<UInt8> Pixels;
	Pixels.resize(HighResolutionSignalSizeY * HighResolutionSignalSizeX * 4, 128);

	for (int i = -HighResolutionSignalSizeY / 2; i < HighResolutionSignalSizeY / 2; i++)
	{
		for (int j = -HighResolutionSignalSizeX / 2; j < HighResolutionSignalSizeX / 2; j++)
		{
			Ray tracingRay;
			tracingRay.Origin = center;
			float ScreenY = ((float)i) / HighResolutionSignalSizeY * ScreenWorldHeight;
			float ScreenX = ((float)j) / HighResolutionSignalSizeX * ScreenWorldWidth;
			float ScreenZ = ScreenNearZ;
			Point ScreenPos(center.x + ScreenNearZ, center.y + ScreenX, center.z - ScreenY);
			Intersection Intersection;
			tracingRay.Direction = ScreenPos - tracingRay.Origin;
			tracingRay.Direction = glm::normalize(tracingRay.Direction);
			if (BVHTree.Intersect(tracingRay, Intersection))
			{
				float IndexY = i + HighResolutionSignalSizeY / 2;
				float IndexX = j + HighResolutionSignalSizeX / 2;
				float PixelsIndex = IndexY * HighResolutionSignalSizeX * 4 + IndexX * 4;
				Pixels[PixelsIndex] = abs(Intersection.Normal.x) * 255;
				Pixels[PixelsIndex + 1] = abs(Intersection.Normal.y) * 255;
				Pixels[PixelsIndex + 2] = abs(Intersection.Normal.z) * 255;
				Pixels[PixelsIndex + 3] = 255;
			}
		}
	}
	
	OutputImageHelper::OutputPNG("tmp.png", Pixels, HighResolutionSignalSizeX, HighResolutionSignalSizeY);
	/*
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
	*/

    return 0;
}
