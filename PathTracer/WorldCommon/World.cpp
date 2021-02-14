#include "World.h"
#include "./../Vulkan/ResourceCreator.h"

extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;
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

	mWorldParams.VoxelDimension = 256;
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
	dimension = 256;
	Matrix view = GetViewMatrix();
	Matrix proj = GetProjMatrix();
	viewProj = proj * view;
	
	voxelSize = (mMaxPoint - mMinPoint)[0] / dimension;
	worldMinPoint = mMinPoint;
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

void World::Update(int frameIndex)
{
	static float theta = 0.0f;
	Vec3 point = Vec3(300.0f, 200.0f, 0.0f);
	mMainCamera.SetParams(((float)gScreenWidth) / gScreenHeight, 0.1f, 10000.0f, glm::radians(60.0f), Vec3(cos(theta), 0.0f, sin(theta)), point, Vec3(0, -1, 0));
	theta += 0.01f;

	float voxelDimension = mWorldParams.VoxelDimension;
	
	Vec3 sceneSize = mMaxPoint - mMinPoint;
	float maxLength = std::max(sceneSize.x, std::max(sceneSize.y, sceneSize.z));
	float voxelSize = maxLength / voxelDimension;
	float halfLength = 0.5f * maxLength;
	Vec3 center = 0.5f * (mMinPoint + mMaxPoint);
	Matrix projection = glm::ortho(-halfLength, halfLength, -halfLength, halfLength, -maxLength, maxLength);

	Matrix orthViewProjMatrix[3];
	Matrix orthViewProjInverseMatrix[3];
	orthViewProjMatrix[0] = glm::lookAt(center + glm::vec3(halfLength, 0.0f, 0.0f), center, glm::vec3(0.0f, 1.0f, 0.0f));
	orthViewProjMatrix[1] = glm::lookAt(center + glm::vec3(0.0f, halfLength, 0.0f), center, glm::vec3(0.0f, 0.0f, -1.0f));
	orthViewProjMatrix[2] = glm::lookAt(center + glm::vec3(0.0f, 0.0f, halfLength), center, glm::vec3(0.0f, 1.0f, 0.0f));
	Matrix view, proj;
	mMainCamera.GetViewTransformMatrix(view, proj);

	for (int i = 0; i < 3; i++)
	{
		orthViewProjMatrix[i] = projection * orthViewProjMatrix[i];
		orthViewProjInverseMatrix[i] = glm::inverse(orthViewProjMatrix[i]);
	}

	for (auto& materialParam : mWorldMaterialParams)
	{
		bool needUpdate = false;
		auto SetParam = [&needUpdate](auto& param, std::string name, auto value)->void
		{
			if (param.first == name)
			{
				param.second.Value = value;
				needUpdate = true;
			}
		};

		auto& params = materialParam.second.Params;
		for (auto& floatParam : params.FloatParams)
		{
			SetParam(floatParam, "voxelSize", voxelSize);
			SetParam(floatParam, "voxelDimension", voxelDimension);
		}

		for (auto& float3Param : params.Vec3Params)
		{
			SetParam(float3Param, "worldMinPoint", mMinPoint);
		}

		for (auto& matParam : params.MatrixParams)
		{
			SetParam(matParam, "DirectionViewProjection[0]", orthViewProjMatrix[0]);
			SetParam(matParam, "DirectionViewProjection[1]", orthViewProjMatrix[1]);
			SetParam(matParam, "DirectionViewProjection[2]", orthViewProjMatrix[2]);
			SetParam(matParam, "DirectionInverseViewProjection[0]", orthViewProjInverseMatrix[0]);
			SetParam(matParam, "DirectionInverseViewProjection[1]", orthViewProjInverseMatrix[1]);
			SetParam(matParam, "DirectionInverseViewProjection[2]", orthViewProjInverseMatrix[2]);
			SetParam(matParam, "view", view);
			SetParam(matParam, "proj", proj);
		}

		if (needUpdate)
		{
			for (auto param : materialParam.second.Params.Vec4Params)
				UpdateConstantBufferParam(frameIndex, param.second.Offset, materialParam.second.CBuffer[frameIndex][param.second.Binding], param.second.Value);
			for (auto param : materialParam.second.Params.Vec3Params)
				UpdateConstantBufferParam(frameIndex, param.second.Offset, materialParam.second.CBuffer[frameIndex][param.second.Binding], param.second.Value);
			for (auto param : materialParam.second.Params.FloatParams)
				UpdateConstantBufferParam(frameIndex, param.second.Offset, materialParam.second.CBuffer[frameIndex][param.second.Binding], param.second.Value);
			for (auto param : materialParam.second.Params.MatrixParams)
				UpdateConstantBufferParam(frameIndex, param.second.Offset, materialParam.second.CBuffer[frameIndex][param.second.Binding], param.second.Value);
		}
	}

}

void World::AddMaterialParams(VulkanMaterial::MaterialPtr material)
{
	static const std::vector<std::string> sNeedMipInnerTextureName = 
	{
		"voxelMipMapAlbedo_IN"
	};
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
				{
					size = mWorldParams.VoxelDimension;
					if (image.first.find("voxelMipMapAlbedo_IN") != image.first.npos)
					{
						size = mWorldParams.VoxelDimension >> 1;
					}
				}
				ImageDesc desc = {};
				desc.ArrayLayers = 1;
				desc.Dimension = image.second.ImageDimension;
				desc.Format = ResourceCreator::GetInnerImageDataFormat(image.second.Format);
				desc.GenerateMipMap = false;
				desc.MipLevels = 1;
				desc.Usage = TextureUsageBits::TU_STORAGE | TextureUsageBits::TU_SHADER_RESOURCE;
				desc.Width = size;
				desc.Height = size;
				desc.Depth = size;
				desc.ImageData = nullptr;
				for (auto& innerMipStr : sNeedMipInnerTextureName)
				{
					if (innerMipStr.find(image.first) != innerMipStr.npos)
					{
						desc.MipLevels = static_cast<UInt32>(std::floor(std::log2(std::max(desc.Width, desc.Height) + 1)));
					}
				}

				for (int i = 0; i < gSwapChainImageCount; i++)
				{
					materialParams.InnerImage[i][image.second.Binding] = ResourceCreator::CreateInnerImage(desc, image.first, i);
					materialParams.InnerImage[i][image.second.Binding]->AddImageView(image.second.Format);
				}
			}
		}

		std::vector<VkWriteDescriptorSet> descriptorSetWrite;
		std::vector<VkDescriptorImageInfo> descImageInfoVec;
		descriptorSetWrite.reserve(gSwapChainImageCount * materialParams.Params.ImageParams.size());
		descImageInfoVec.reserve(gSwapChainImageCount * materialParams.Params.ImageParams.size());

		for (auto image : materialParams.Params.ImageParams)
		{
			if (image.second.InnerImage)
			{
				for (int i = 0; i < gSwapChainImageCount; i++)
				{
					auto pImage = materialParams.InnerImage[i][image.second.Binding];
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					if (image.second.CombinedSampler)
					{
						imageInfo.sampler = *((VkSampler*)pImage->GetSamplerHandleAddress());
						imageInfo.imageView = *((VkImageView*)pImage->GetGPUImageViewHandleAddress());
					}					
					else
					{
						imageInfo.sampler = VK_NULL_HANDLE;
						imageInfo.imageView = *((VkImageView*)pImage->GetGPUImageViewHandleAddress(image.second.Format, 0));
					}

					descImageInfoVec.push_back(imageInfo);
					VkWriteDescriptorSet descriptorWrite = {};
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = materialParams.DescSet[i];
					descriptorWrite.dstBinding = image.second.Binding;
					descriptorWrite.dstArrayElement = image.second.ArrayIndex;
					if (image.second.CombinedSampler)
					{
						descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
					}
					else
					{
						descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					}
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
