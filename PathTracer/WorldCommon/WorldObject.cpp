#include "WorldObject.h"
#include "./../Vulkan/ResourceCreator.h"

extern UInt32 gSwapChainImageCount;
extern VkDevice gVulkanDevice;

template<typename T>
void UpdateConstantBufferParam(int frameIndex, int offset, IBuffer::BufferPtr buffer, T& value)
{
	char* data;
	int mOffset;
	auto memory = std::dynamic_pointer_cast<VulkanBuffer>(buffer)->GetGPUBufferMemory(mOffset);
	vkMapMemory(gVulkanDevice, memory, mOffset + offset, sizeof(T), 0, &((void*)data));
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
	mStatic = true;
}

WorldObject::~WorldObject()
{
	vkFreeDescriptorSets(gVulkanDevice, mMaterialResource.DescPool, mMaterialResource.DescSet.size(), mMaterialResource.DescSet.data());
	vkDestroyDescriptorPool(gVulkanDevice, mMaterialResource.DescPool, nullptr);
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

void WorldObject::UpdateParams()
{
	if (mMaterialResource.Params.MatrixParams.find("modelTransform") != mMaterialResource.Params.MatrixParams.end())
	{
		Matrix rotation = glm::identity<Matrix>();
		rotation = glm::rotate(rotation, mTransform.Rotation.x, Vec3(1, 0, 0));
		rotation = glm::rotate(rotation, mTransform.Rotation.y, Vec3(0, 1, 0));
		rotation = glm::rotate(rotation, mTransform.Rotation.z, Vec3(0, 0, 1));
		Matrix trans = glm::identity<Matrix>();
		trans = glm::scale(rotation, Vec3(mTransform.Scale.x, mTransform.Scale.y, mTransform.Scale.z));
		trans = glm::translate(trans, Vec3(mTransform.Position.x, mTransform.Position.y, mTransform.Position.z));
		mMaterialResource.Params.MatrixParams["modelTransform"].Value = trans;

		if (mMaterialResource.Params.MatrixParams.find("normalTransform") != mMaterialResource.Params.MatrixParams.end())
		{
			Matrix normal = glm::transpose(glm::inverse(trans));
			mMaterialResource.Params.MatrixParams["normalTransform"].Value = normal;
		}

		if (mStatic)
		{
			mMaterialResource.Params.FloatParams["staticVoxelFlag"].Value = 1.0f;
		}
	}
}

void WorldObject::Update(int frameIndex)
{
	if (mDirty[frameIndex])
	{
		UpdateParams();
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
	if (mMaterialResource.DescSet.size() == 0)
		return VK_NULL_HANDLE;
	return mMaterialResource.DescSet[frameIndex];
}

void WorldObject::InitMaterialResource()
{
	auto material = ResourceCreator::CreateMaterial(mMaterialName, MaterialMode::Normal);
	material->ExportPerObjectDescriptor(mMaterialResource.SetLayout, mMaterialResource.DescPool, mMaterialResource.DescSet, mMaterialResource.Params, mMaterialResource.CBuffer);
}
