#include "World.h"
#include "./../Vulkan/ResourceCreator.h"

extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;
extern UInt32 gSwapChainImageCount;
extern VkDevice gVulkanDevice;

World::World(const ImportSceneData& sceneData)
	:mMainCamera(sceneData.CameraData)
{
	UInt32 lightCount = sceneData.LightData.LightVec.size();
	for (auto i = 0u; i < lightCount; i++)
	{
		mLights[i] = Light(sceneData.LightData.LightVec[i]);
	}

	if (lightCount == 0)
	{
		SetCameraDefault(sceneData);
		AddDefaultDirectionalLight(0);
	}
	auto meshCount = sceneData.MeshData.MinPointVec.size();
	mMinPoint = Vec3(MAXF, MAXF, MAXF);
	mMaxPoint = Vec3(-MAXF, -MAXF, -MAXF);
	for (auto i = 0; i < meshCount; i++)
	{
		mMinPoint = glm::min(sceneData.MeshData.MinPointVec[i], mMinPoint);
		mMaxPoint = glm::max(sceneData.MeshData.MaxPointVec[i], mMaxPoint);
	}

	mWorldParams.VoxelDimension = 64;
}

World::~World()
{
	for (auto& materialParams : mWorldMaterialParams)
	{
		vkFreeDescriptorSets(gVulkanDevice, materialParams.second.DescPool, materialParams.second.DescSet.size(), materialParams.second.DescSet.data());
		vkDestroyDescriptorPool(gVulkanDevice, materialParams.second.DescPool, nullptr);
	}
}

void World::SetCameraDefault(const ImportSceneData& sceneData)
{
	Vec3 point = Vec3(0.0f, 400.0f, 0.0f);
	mMainCamera.SetParams(((float)gScreenWidth) / gScreenHeight, 0.1f, 10000.0f, 1.0f, Vec3(0.0f, -1.0f, 0.0f), point, Vec3(0, 0, 1));
}

void World::AddDefaultDirectionalLight(int index)
{
	mLights[index].mDirection = glm::normalize(Vec3(1, 1, 1));
	mLights[index].mDiffuse = Vec3(1, 1, 1);
	mLights[index].mSpecular = Vec3(1, 1, 1);
	mLights[index].mAmbient = Vec3(1, 1, 1);
	mLights[index].mType = LightType::LT_DirectionalLight;
}

void World::GetVoxelizationParams(int& dimension, Matrix & viewProj, float& voxelSize, Vec3 & worldMinPoint)
{
	dimension = 64;
	Matrix view = GetViewMatrix();
	Matrix proj = GetProjMatrix();
	viewProj = proj * view;
	
	voxelSize = (mMaxPoint - mMinPoint)[0] / dimension;
	worldMinPoint = mMinPoint;
	worldMinPoint = Vec3(300.0f, 600.0f, 100.0f);
}

Light World::GetLight(int index)
{
	return mLights[index];
}

Matrix World::GetViewMatrix()
{
	Matrix view, proj;
	mMainCamera.GetViewTransformMatrix(view, proj);
	return view;
}

Matrix World::GetProjMatrix()
{
	Matrix view, proj;
	mMainCamera.GetViewTransformMatrix(view, proj);
	return proj;
}

void World::Update()
{
	static float theta = 0.0f;
	Vec3 point = Vec3(300.0f, 600.0f, 0.0f);
	mMainCamera.SetParams(((float)gScreenWidth) / gScreenHeight, 0.1f, 10000.0f, glm::radians(60.0f), Vec3(sin(theta), 0.0f, cos(theta)), point, Vec3(0, -1, 0));
	theta += 0.01f;
}

void World::AddMaterialParams(VulkanMaterial::MaterialPtr material)
{
	auto matName = material->GetShaderGroupName();
	if (mWorldMaterialParams.find(matName) == mWorldMaterialParams.end())
	{
		auto& materialParams = mWorldMaterialParams[matName];
		material->ExportPerCameraDescriptor(
			materialParams.SetLayout,
			materialParams.DescPool,
			materialParams.DescSet,
			materialParams.Params,
			materialParams.CBuffer
		);

		materialParams.InnerImage.resize(gSwapChainImageCount);

		for (auto image : materialParams.Params.ImageParams)
		{
			if (image.second.InnerImage)
			{
				int size = 0;
				if (image.first.find("voxel") != image.first.npos || image.first.find("Voxel") != image.first.npos)
					size = mWorldParams.VoxelDimension;
				ImageDesc desc = {};
				desc.ArrayLayers = 1;
				desc.Dimension = image.second.ImageDimension;
				desc.Format = ResourceCreator::GetInnerImageDataFormat(image.second.Format);
				desc.GenerateMipMap = false;
				desc.MipLevels = 1;
				desc.Usage = TextureUsageBits::TU_SHADER_RESOURCE;
				desc.Width = size;
				desc.Height = size;
				desc.Depth = size;
				desc.ImageData = nullptr;

				for (int i = 0; i < gSwapChainImageCount; i++)
				{
					materialParams.InnerImage[i][image.second.Binding] = std::make_shared<VulkanImage>(desc);
				}
			}
		}

		std::vector<VkWriteDescriptorSet> descriptorSetWrite;
		std::vector<VkDescriptorImageInfo> descImageInfoVec;

		for (auto image : materialParams.Params.ImageParams)
		{
			if (image.second.InnerImage)
			{
				for (int i = 0; i < gSwapChainImageCount; i++)
				{
					auto pImage = materialParams.InnerImage[i][image.second.Binding];
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = *((VkImageView*)pImage->GetGPUImageViewHandleAddress());
					imageInfo.sampler = *((VkSampler*)pImage->GetSamplerHandleAddress());
					descImageInfoVec.push_back(imageInfo);

					VkWriteDescriptorSet descriptorWrite = {};
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = materialParams.DescSet[i];
					descriptorWrite.dstBinding = image.second.Binding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.pBufferInfo = nullptr;
					descriptorWrite.pImageInfo = &descImageInfoVec.back();
					descriptorWrite.pTexelBufferView = nullptr;
					descriptorSetWrite.push_back(descriptorWrite);
				}
			}
		}
		vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
	}
}

VkDescriptorSet World::GetWorldParamsSet(VulkanMaterial * material, int frameIndex)
{
	std::string shaderGroupName = material->GetShaderGroupName();
	if (mWorldMaterialParams.find(shaderGroupName) == mWorldMaterialParams.end() || mWorldMaterialParams[shaderGroupName].DescSet.size() == 0)
		return VK_NULL_HANDLE;
	return mWorldMaterialParams[shaderGroupName].DescSet[frameIndex];
}
