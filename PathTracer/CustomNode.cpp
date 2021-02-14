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
	Vec3 localSize = mMaterial->GetComputeLocalSize();
	int groupSizeX = std::ceilf(mInvocation.x / localSize.x);
	int groupSizeY = std::ceilf(mInvocation.y / localSize.y);
	int groupSizeZ = std::ceilf(mInvocation.z / localSize.z);
	auto mipMapImage = ResourceCreator::CreateInnerImage(ImageDesc(), "voxelMipMapAlbedo_IN[0]", 0);
	int mipMapCount = mipMapImage->GetMipMapLevelCount();

	for (int i = 0; i < mipMapCount; i ++)
	{
		int descCount;
		std::vector<VkDescriptorSet> submitDescSetVec = mMaterial->GetDescriptorSet(frameIndex, descCount, true);
		VkDescriptorSet worldSet = mCameraMaterialResource[i].DescSet[frameIndex];
		submitDescSetVec.push_back(worldSet);
		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, submitDescSetVec.size(), submitDescSetVec.data(), 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);
		vkCmdDispatch(commandBuffer, std::min(1, groupSizeX), std::min(1, groupSizeY), std::min(1, groupSizeZ));

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
	}

}

void VoxelMipMapCustomNode::CreateMipMapDescriptorSet()
{
	auto mipMapImage = ResourceCreator::CreateInnerImage(ImageDesc(), "voxelMipMapAlbedo_IN[0]", 0);
	auto baseImage = ResourceCreator::CreateInnerImage(ImageDesc(), "voxelMipMapAlbedoBase_IN[0]", 0);
	int mipMapCount = mipMapImage->GetMipMapLevelCount();
	mCameraMaterialResource.reserve(mipMapCount);
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
					mipMapBaseImageName = std::string("voxelMipMapAlbedoBase_IN[") + std::to_string(arrayIndex) + std::string("]");
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
				descriptorSetWrite.push_back(descriptorWrite);
			}

			vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
		}

	}
}
