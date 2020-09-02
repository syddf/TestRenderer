#include "Image.h"
#include "TranslateEngine.h"
#include "ResourceCreator.h"
#include "ImageView.h"

extern VkDevice gVulkanDevice;
extern ITranslationEngine* gTranslateEngine;
extern VulkanCommandBufferPool* gGraphicsCommonCommandBufferPool;

VulkanImage::VulkanImage(ImageDesc desc)
{
	CreateImage(desc);
}

VulkanImage::~VulkanImage()
{
	vkFreeMemory(gVulkanDevice, mImageMemory, nullptr);
	vkDestroyImage(gVulkanDevice, mImage, nullptr);
	vkDestroySampler(gVulkanDevice, mSampler, nullptr);
}

void VulkanImage::CreateImage(ImageDesc desc)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.format = GetVKTextureFormat(desc.Format);
	imageCreateInfo.imageType = GetVKTextureType(desc.Dimension);
	imageCreateInfo.usage = GetVKImageUsageFlags(desc.Usage);
	imageCreateInfo.initialLayout = desc.Usage == TextureUsageBits::TU_DEPTH_STENCIL ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent.width = desc.Width;
	imageCreateInfo.extent.height = desc.Height;
	imageCreateInfo.extent.depth = desc.Depth;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.mipLevels = desc.MipLevels;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.arrayLayers = desc.ArrayLayers;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	VKFUNC(vkCreateImage(gVulkanDevice, &imageCreateInfo, nullptr, &mImage), "Create Image Failed.");

	mImageLayout = imageCreateInfo.initialLayout;

	VkMemoryRequirements memoryRequirement;
	vkGetImageMemoryRequirements(gVulkanDevice, mImage, &memoryRequirement);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirement.size;
	allocInfo.memoryTypeIndex = GetVKMemoryType(memoryRequirement.memoryTypeBits, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkAllocateMemory(gVulkanDevice, &allocInfo, nullptr, &mImageMemory);
	vkBindImageMemory(gVulkanDevice, mImage, mImageMemory, 0);

	if (desc.ImageData)
	{
		size_t bufferSize = GetVKTextureSize(desc);
		IBuffer::BufferPtr stagingBuffer = ResourceCreator::CreateStagingBuffer(desc.ImageData, bufferSize);
		TranslateImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, desc.MipLevels);
		VulkanTranslateEngine* transEngine = dynamic_cast<VulkanTranslateEngine*>(gTranslateEngine);
		TranslateBufferToImageDesc transDesc = {};
		transDesc.SrcBufferOffset = 0;
		transDesc.GPUBufferHandlePtr = stagingBuffer->GetGPUBufferHandleAddress();
		transDesc.GPUImageHandlePtr = GetGPUImageHandleAddress();
		transDesc.DstTextureOffset[0] = 0;
		transDesc.DstTextureOffset[1] = 0;
		transDesc.DstTextureOffset[2] = 0;
		transDesc.DstTextureMipLevel = 0;
		transDesc.DstTextureLayerCount = imageCreateInfo.arrayLayers;
		transDesc.DstTextureBaseLayer = 0;
		transDesc.DstTextureAspect = GetTextureAspectFlagBits(desc.Usage);
		transDesc.DstTextureExtent[0] = desc.Width;
		transDesc.DstTextureExtent[1] = desc.Height;
		transDesc.DstTextureExtent[2] = desc.Depth;
		transEngine->TranslateBufferToTexture(transDesc);

		if (desc.GenerateMipMap)
		{
			GenerateMipMap(desc.Width, desc.Height, desc.MipLevels);
		}
	}

	ImageViewDesc viewDesc = {};
	viewDesc.Format = desc.Format;
	viewDesc.IsColorTexture = (desc.Usage & TU_DEPTH_STENCIL) ? false : true;
	viewDesc.Dimension = desc.Dimension;
	viewDesc.BaseArrayLayer = 0;
	viewDesc.LayerCount = desc.ArrayLayers;
	viewDesc.BaseMipLevel = 0;
	viewDesc.MipLevelCount = desc.MipLevels;
	viewDesc.ImageHandleAddr = reinterpret_cast<char*>(&mImage);
	mImageView = std::make_shared<VulkanImageView>(viewDesc);

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VKFUNC(vkCreateSampler(gVulkanDevice, &samplerInfo, nullptr, &mSampler), "Failed To Create Sampler");
}

void VulkanImage::TranslateImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkImageAspectFlags aspect, UInt32 mipLevels)
{
	if (newLayout == mImageLayout)
		return;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = mImageLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = mImage;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && barrier.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		throw "Unexpected layout transition";
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	mImageLayout = newLayout;
}

void VulkanImage::TranslateImageLayout(VkImageLayout newLayout, VkImageAspectFlags aspect, UInt32 mipLevels)
{
	if (newLayout == mImageLayout)
		return;
	VkCommandBuffer commandBuffer = gGraphicsCommonCommandBufferPool->BeginSingleTimeCommandBuffer(true);
	TranslateImageLayout(commandBuffer, newLayout, aspect, mipLevels);
	gGraphicsCommonCommandBufferPool->EndSingleTimeCommandBuffer(commandBuffer);
}

void VulkanImage::GenerateMipMap(UInt32 Width, UInt32 Height, UInt32 MipLevels)
{
	VkCommandBuffer genMipMapCommand = gGraphicsCommonCommandBufferPool->BeginSingleTimeCommandBuffer(true);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = mImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	UInt32 mipWidth = Width;
	UInt32 mipHeight = Height;
	for (UInt32 i = 1; i < MipLevels; i++)
	{
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.subresourceRange.baseMipLevel = i - 1;

		vkCmdPipelineBarrier(genMipMapCommand, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 0, 0, 1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { (int)mipWidth, (int)mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { (int)mipWidth > 1 ? (int)mipWidth / 2 : 1, (int)mipHeight > 1 ? (int)mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			genMipMapCommand,
			mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(genMipMapCommand, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = MipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(genMipMapCommand,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	gGraphicsCommonCommandBufferPool->EndSingleTimeCommandBuffer(genMipMapCommand);
}

char* VulkanImage::GetGPUImageHandleAddress()
{
	return reinterpret_cast<char*>(&mImage);
}

char * VulkanImage::GetGPUImageViewHandleAddress()
{
	return mImageView->GetGPUImageViewHandleAddress();
}

char * VulkanImage::GetSamplerHandleAddress()
{
	return reinterpret_cast<char*>(&mSampler);
}
