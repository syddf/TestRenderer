#pragma once
#include "./../../Source/Prefix.h"
#include "Camera.h"
#include "Light.h"
#include "WorldObject.h"
#define MAX_LIGHT 256

struct WorldMaterialResource
{
	VkDescriptorSetLayout SetLayout;
	VkDescriptorPool DescPool;
	std::vector<VkDescriptorSet> DescSet;
	MaterialParams Params;
	std::vector<std::map<int, IBuffer::BufferPtr>> CBuffer;
	std::vector<std::map<int, IImage::ImagePtr>> InnerImage;
};

struct WorldParams
{
	int VoxelDimension;
};

class World
{
public:
	World(const ImportSceneData& sceneData);
	~World();

private:
	Camera mMainCamera;
	Light mLights[MAX_LIGHT];
	Vec3 mMinPoint;
	Vec3 mMaxPoint;

private:
	void SetCameraDefault(const ImportSceneData& sceneData);
	void AddDefaultDirectionalLight(int index);
	
public:
	Vec3 GetCameraPosition() const { return mMainCamera.GetPosition(); }
	void GetVoxelizationParams(int& dimension, Matrix& viewProj, float& voxelSize, Vec3& worldMinPoint);
	Light GetLight(int index);
	Matrix GetViewMatrix();
	Matrix GetProjMatrix();
	void Update(int frameIndex);
	void AddMaterialParams(VulkanMaterial::MaterialPtr material);
	VkDescriptorSet GetWorldParamsSet(VulkanMaterial* material, int frameIndex);

private:
	std::map<std::string, WorldMaterialResource> mWorldMaterialParams;
	WorldParams mWorldParams;
};