#include "CustomNode.h"
#include "Vulkan/ResourceCreator.h"

extern UInt32 gSwapChainImageCount;
extern VkDevice gVulkanDevice;

VoxelMipMapCustomNode::VoxelMipMapCustomNode(ComputeNodeDesc desc)
	:VulkanComputeNode(desc)
{
	CreateMipMapDescriptorSet();
}

VoxelMipMapCustomNode::~VoxelMipMapCustomNode()
{
	for (auto& materialParams : mCameraMaterialResource)
	{
		vkFreeDescriptorSets(gVulkanDevice, materialParams.DescPool, materialParams.DescSet.size(), materialParams.DescSet.data());
		vkDestroyDescriptorPool(gVulkanDevice, materialParams.DescPool, nullptr);
	}
}

void VoxelMipMapCustomNode::RecordCommandBuffer(VkCommandBuffer & commandBuffer, int frameIndex)
{
	struct MipMapPushConstantData
	{
		int mipLevel;
		int mipDimension;
	}pushData;

	Vec3 localSize = mMaterial->GetComputeLocalSize();
	int groupSizeX = std::ceilf(mInvocation.x / localSize.x);
	int groupSizeY = std::ceilf(mInvocation.y / localSize.y);
	int groupSizeZ = std::ceilf(mInvocation.z / localSize.z);
	auto mipMapImage = ResourceCreator::CreateInnerImage(ImageDesc(), "voxelMipMapAlbedo_IN[0]", 0);
	int mipMapCount = mipMapImage->GetMipMapLevelCount();
	pushData.mipDimension = mInvocation.x;

	for (int i = 0; i < mipMapCount; i ++)
	{
		pushData.mipLevel = i;
		int descCount;
		std::vector<VkDescriptorSet> submitDescSetVec = mMaterial->GetDescriptorSet(frameIndex, descCount, true);
		VkDescriptorSet worldSet = mCameraMaterialResource[i].DescSet[frameIndex];
		submitDescSetVec.push_back(worldSet);
		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, submitDescSetVec.size(), submitDescSetVec.data(), 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);
		vkCmdPushConstants(commandBuffer, mPipelineLayout, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 0, 8, &pushData);
		vkCmdDispatch(commandBuffer, std::max(1, groupSizeX), std::max(1, groupSizeY), std::max(1, groupSizeZ));

		VkMemoryBarrier memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		if (i < mipMapCount - 1)
		{
			vkCmdPipelineBarrier(
				commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0,
				1,
				&memoryBarrier,
				0,
				nullptr,
				0,
				nullptr);
		}
		groupSizeX = groupSizeX >> 1;
		groupSizeY = groupSizeY >> 1;
		groupSizeZ = groupSizeZ >> 1;
		pushData.mipDimension = pushData.mipDimension >> 1;
	}

}

void VoxelMipMapCustomNode::CreateMipMapDescriptorSet()
{
	auto mipMapImage = ResourceCreator::CreateInnerImage(ImageDesc(), "voxelMipMapAlbedo_IN[0]", 0);
	auto baseImage = ResourceCreator::CreateInnerImage(ImageDesc(), "voxelMipMapAlbedoBase_IN[0]", 0);
	int mipMapCount = mipMapImage->GetMipMapLevelCount();
	mCameraMaterialResource.resize(mipMapCount);
	for (int mipMapLevel = 0; mipMapLevel < mipMapCount; mipMapLevel ++)
	{
		mMaterial->ExportPerCameraDescriptor
		(
			mCameraMaterialResource[mipMapLevel].SetLayout,
			mCameraMaterialResource[mipMapLevel].DescPool,
			mCameraMaterialResource[mipMapLevel].DescSet,
			mCameraMaterialResource[mipMapLevel].Params,
			mCameraMaterialResource[mipMapLevel].CBuffer
		);

		for (int frameIndex = 0; frameIndex < gSwapChainImageCount; frameIndex ++)
		{
			std::vector<VkWriteDescriptorSet> descriptorSetWrite;
			std::vector<VkDescriptorImageInfo> descImageInfoVec;
			descriptorSetWrite.reserve(gSwapChainImageCount * mCameraMaterialResource[mipMapLevel].Params.ImageParams.size());
			descImageInfoVec.reserve(gSwapChainImageCount * mCameraMaterialResource[mipMapLevel].Params.ImageParams.size());

			for (int arrayIndex = 0; arrayIndex < 6; arrayIndex++)
			{
				std::string mipMapAlbedoImageName = std::string("voxelMipMapAlbedo_IN[") + std::to_string(arrayIndex) + std::string("]");
				auto mipMapImage = ResourceCreator::CreateInnerImage(ImageDesc(), mipMapAlbedoImageName, frameIndex);

				std::string mipMapBaseImageName;
				if (mipMapLevel == 0)
				{
					mipMapBaseImageName = std::string("voxelMipMapAlbedoBase_IN[5]");
				}
				else
				{
					mipMapBaseImageName = mipMapAlbedoImageName;
				}

				auto mipMapBaseImage = ResourceCreator::CreateInnerImage(ImageDesc(), mipMapBaseImageName, frameIndex);

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				imageInfo.sampler = VK_NULL_HANDLE;
				imageInfo.imageView = *((VkImageView*)mipMapImage->GetGPUImageViewHandleAddress("Rgba8", mipMapLevel));

				descImageInfoVec.push_back(imageInfo);
				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mCameraMaterialResource[mipMapLevel].DescSet[frameIndex];
				descriptorWrite.dstBinding = 0;
				descriptorWrite.dstArrayElement = arrayIndex;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = nullptr;
				descriptorWrite.pImageInfo = &descImageInfoVec.back();
				descriptorWrite.pTexelBufferView = nullptr;
				descriptorSetWrite.push_back(descriptorWrite);

				imageInfo.sampler = *((VkSampler*)mipMapBaseImage->GetSamplerHandleAddress());
				imageInfo.imageView = *((VkImageView*)mipMapBaseImage->GetGPUImageViewHandleAddress("Rgba8", mipMapLevel == 0 ? 0 : mipMapLevel - 1));
				descImageInfoVec.push_back(imageInfo);
				descriptorWrite.dstBinding = 1;
				descriptorWrite.pImageInfo = &descImageInfoVec.back();
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorSetWrite.push_back(descriptorWrite);
			}

			vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
		}

	}
}

BlurCustomNode::BlurCustomNode(VulkanMaterial::MaterialPtr material, World* world, std::vector<IImage::ImagePtr> srcImage, std::vector<IImage::ImagePtr> blurImage, Vec2 blurDirection)
{
	mMaterial = material;
	mWorld = world;
	mSrcImage = srcImage;
	mBlurImage = blurImage;
	mBlurDirection = blurDirection;
	CreateQuadMesh();
	CreateResources();
}

void BlurCustomNode::RecordCommandBuffer(VkCommandBuffer & commandBuffer, int frameIndex, VkPipelineLayout pipelineLayout)
{
	auto mesh = ResourceCreator::GetExportedMesh("Quad");
	VkBuffer* vertBuffer = reinterpret_cast<VkBuffer*>(mesh->GetVertexBufferGPUHandleAddress());
	VkBuffer* indBuffer = reinterpret_cast<VkBuffer*>(mesh->GetIndexBufferGPUHandleAddress());

	std::dynamic_pointer_cast<VulkanImage>(mSrcImage[frameIndex])->TranslateImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	std::dynamic_pointer_cast<VulkanImage>(mBlurImage[frameIndex])->TranslateImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	VkDeviceSize offsets[] = { 0 };

	int descCount;
	std::vector<VkDescriptorSet> submitDescSetVec = mMaterial->GetDescriptorSet(frameIndex, descCount, true);
	VkDescriptorSet worldSet = mCameraMaterialResource.DescSet[frameIndex];
	submitDescSetVec.push_back(worldSet);

	vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, submitDescSetVec.size(), submitDescSetVec.data(), 0, nullptr);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertBuffer, offsets);
	vkCmdPushConstants(commandBuffer, pipelineLayout, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, 0, 8, &mBlurDirection);
	vkCmdBindIndexBuffer(commandBuffer, *indBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

void BlurCustomNode::CreateResources()
{
	VkSampler samplers[3] = 
	{ 
		*((VkSampler*)mSrcImage[0]->GetSamplerHandleAddress()),
		*((VkSampler*)mSrcImage[1]->GetSamplerHandleAddress()),
		*((VkSampler*)mSrcImage[2]->GetSamplerHandleAddress())
	};

	VkImageView imageViews[3] = 
	{
		*((VkImageView*)mSrcImage[0]->GetGPUImageViewHandleAddress()),
		*((VkImageView*)mSrcImage[1]->GetGPUImageViewHandleAddress()),
		*((VkImageView*)mSrcImage[2]->GetGPUImageViewHandleAddress())
	};

	//for (int i = 0; i < 2; i ++)
	{
		mMaterial->ExportPerCameraDescriptor
		(
			mCameraMaterialResource.SetLayout,
			mCameraMaterialResource.DescPool,
			mCameraMaterialResource.DescSet,
			mCameraMaterialResource.Params,
			mCameraMaterialResource.CBuffer
		);

		for (int frameIndex = 0; frameIndex < gSwapChainImageCount; frameIndex++)
		{
			std::vector<VkWriteDescriptorSet> descriptorSetWrite;
			std::vector<VkDescriptorImageInfo> descImageInfoVec;
			descriptorSetWrite.reserve(mCameraMaterialResource.Params.ImageParams.size());
			descImageInfoVec.reserve(mCameraMaterialResource.Params.ImageParams.size());

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = samplers[frameIndex];
			imageInfo.imageView = imageViews[frameIndex];
			descImageInfoVec.push_back(imageInfo);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mCameraMaterialResource.DescSet[frameIndex];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = &descImageInfoVec.back();
			descriptorWrite.pTexelBufferView = nullptr;
			descriptorSetWrite.push_back(descriptorWrite);

			vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
		}
	}
}

void BlurCustomNode::CreateQuadMesh()
{
	DefaultMeshStructure defaultMesh = {};

	std::vector<DefaultMeshStructure> vertVec;
	vertVec.resize(4, defaultMesh);

	vertVec[0].TexCoord = Vec2(0.0f, 1.0f);
	vertVec[1].TexCoord = Vec2(1.0f, 1.0f);
	vertVec[2].TexCoord = Vec2(0.0f, 0.0f);
	vertVec[3].TexCoord = Vec2(1.0f, 0.0f);

	vertVec[0].Position = Vec3(-1.0f, 1.0f, 0.1f);
	vertVec[1].Position = Vec3(1.0f, 1.0f, 0.1f);
	vertVec[2].Position = Vec3(-1.0f, -1.0f, 0.1f);
	vertVec[3].Position = Vec3(1.0f, -1.0f, 0.1f);

	std::vector<UInt16> indVec;
	indVec.resize(6);
	indVec[0] = 0;
	indVec[1] = 1;
	indVec[2] = 2;
	indVec[3] = 1;
	indVec[4] = 2;
	indVec[5] = 3;

	std::vector<char> vertCharVec, indCharVec;
	vertCharVec.resize(vertVec.size() * sizeof(DefaultMeshStructure));
	indCharVec.resize(indVec.size() * sizeof(UInt16));

	memcpy(vertCharVec.data(), vertVec.data(), vertCharVec.size());
	memcpy(indCharVec.data(), indVec.data(), indCharVec.size());

	ResourceCreator::CreateInnerMesh("Quad", vertCharVec, indCharVec, 4, 6);
}
