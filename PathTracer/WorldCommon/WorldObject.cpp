#include "WorldObject.h"
#include "./../Vulkan/ResourceCreator.h"

extern UInt32 gSwapChainImageCount;
extern VkDevice gVulkanDevice;

template<typename T>
void UpdateConstantBufferParam(int frameIndex, int offset, IBuffer::BufferPtr buffer, T& value)
{
	char* data;
	auto memory = std::dynamic_pointer_cast<VulkanBuffer>(buffer)->GetGPUBufferMemory();
	vkMapMemory(gVulkanDevice, memory, offset, sizeof(T), 0, &((void*)data));
	memcpy(data, &value, sizeof(T));
	vkUnmapMemory(gVulkanDevice, memory);
}


WorldObject::WorldObject(std::string materialName, std::string modelName, std::string objectName)
{
	mMaterialName = materialName;
	mModelName = modelName;
	mObjectName = objectName;
	InitMaterialResource();
	mDirty.resize(3, false);
}

WorldObject::~WorldObject()
{
}

void WorldObject::SetPosition(float x, float y, float z)
{
	mTransform.Position = Vec3(x, y, z);
	for (int i = 0; i < gSwapChainImageCount; i++)
		mDirty[i] = true;
}

void WorldObject::SetRotation(float x, float y, float z)
{
	mTransform.Rotation = Vec3(x, y, z);
	for (int i = 0; i < gSwapChainImageCount; i++)
		mDirty[i] = true;
}

void WorldObject::SetScale(float x, float y, float z)
{
	mTransform.Scale = Vec3(x, y, z);
	for (int i = 0; i < gSwapChainImageCount; i++)
		mDirty[i] = true;
}

void WorldObject::Update(int frameIndex)
{
	auto material = ResourceCreator::CreateMaterial(mMaterialName);
	material->Update(frameIndex);
	if (mDirty[frameIndex])
	{
		mDirty[frameIndex] = false;
		for (auto param : mMaterialResource.Params.Vec4Params)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialResource.CBuffer[frameIndex][param.second.Binding], param.second.Value);
		for (auto param : mMaterialResource.Params.Vec3Params)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialResource.CBuffer[frameIndex][param.second.Binding], param.second.Value);
		for (auto param : mMaterialResource.Params.FloatParams)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialResource.CBuffer[frameIndex][param.second.Binding], param.second.Value);
		for (auto param : mMaterialResource.Params.MatrixParams)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialResource.CBuffer[frameIndex][param.second.Binding], param.second.Value);
	}
}

IMesh::MeshPtr WorldObject::GetMeshPtr()
{
	return ResourceCreator::GetExportedMesh(mModelName);
}

VkDescriptorSet WorldObject::GetDescSet(int frameIndex)
{
	return mMaterialResource.DescSet[frameIndex];
}

void WorldObject::InitMaterialResource()
{
	auto material = ResourceCreator::CreateMaterial(mMaterialName);
	material->ExportPerObjectDescriptor(mMaterialResource.SetLayout, mMaterialResource.DescPool, mMaterialResource.DescSet, mMaterialResource.Params, mMaterialResource.CBuffer);
}
