#include "TranslateEngine.h"

extern VkDevice gVulkanDevice;

VulkanTranslateEngine::~VulkanTranslateEngine()
{
}

VulkanTranslateEngine::VulkanTranslateEngine(UInt32 TranslateQueueIndex, VkQueue translateQueue)
{
	mCommandPool = std::make_shared<VulkanCommandBufferPool>(TranslateQueueIndex, true, false, translateQueue);
}

void VulkanTranslateEngine::TranslateBufferToTexture(TranslateBufferToImageDesc desc)
{
	VkCommandBuffer commandBuffer = mCommandPool->BeginSingleTimeCommandBuffer(true);

	VkBufferImageCopy bufferImageCopy = {};
	bufferImageCopy.bufferOffset = desc.SrcBufferOffset;
	bufferImageCopy.imageSubresource.aspectMask = GetVKAspectFlagBits(desc.DstTextureAspect);
	bufferImageCopy.imageSubresource.baseArrayLayer = desc.DstTextureBaseLayer;
	bufferImageCopy.imageSubresource.layerCount = desc.DstTextureLayerCount;
	bufferImageCopy.imageSubresource.mipLevel = desc.DstTextureMipLevel;
	bufferImageCopy.imageOffset.x = desc.DstTextureOffset[0];
	bufferImageCopy.imageOffset.y = desc.DstTextureOffset[1];
	bufferImageCopy.imageOffset.z = desc.DstTextureOffset[2];
	bufferImageCopy.imageExtent.width = desc.DstTextureExtent[0];
	bufferImageCopy.imageExtent.height = desc.DstTextureExtent[1];
	bufferImageCopy.imageExtent.depth = desc.DstTextureExtent[2];

	VkBuffer srcBuffer = *reinterpret_cast<VkBuffer*>(desc.GPUBufferHandlePtr);
	VkImage dstImage = *reinterpret_cast<VkImage*>(desc.GPUImageHandlePtr);
	vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	mCommandPool->EndSingleTimeCommandBuffer(commandBuffer);
}

