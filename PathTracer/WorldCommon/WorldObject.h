#pragma once
#include "./../../Source/Prefix.h"
#include "./../Vulkan/Material.h"

struct ObjectTransform
{
	Vec3 Position;
	Vec3 Rotation;
	Vec3 Scale;
};

struct WorldObjectMaterialResource
{
	VkDescriptorSetLayout SetLayout;
	VkDescriptorPool DescPool;
	std::vector<VkDescriptorSet> DescSet;
	MaterialParams Params;
	std::vector<std::map<int, IBuffer::BufferPtr>> CBuffer;
};

struct WorldObject
{
public:
	WorldObject(std::string materialName, std::string modelName, std::string objectName);
	~WorldObject();

	using ObjectPtr = std::shared_ptr<WorldObject>;

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

public:
	std::string mObjectName;
	std::string mMaterialName;
	std::string mModelName;
	int mMeshIndex;
	ObjectTransform mTransform;
	
	void UpdateParams();

	void Update(int frameIndex);
	IMesh::MeshPtr GetMeshPtr();
	VkDescriptorSet GetDescSet(int frameIndex);

private:
	void InitMaterialResource();
	WorldObjectMaterialResource mMaterialResource;
	std::vector<bool> mDirty;
	bool mStatic;
};