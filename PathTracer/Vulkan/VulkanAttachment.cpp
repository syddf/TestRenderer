#include "VulkanAttachment.h"
extern UInt32 gSwapChainImageCount;

VulkanAttachment::VulkanAttachment(ImageDesc desc, bool bIsColor)
{
	mImages.reserve(gSwapChainImageCount);
	mIsColor = bIsColor;
	for (int i = 0; i < gSwapChainImageCount; i ++)
	{
		mImages.push_back(std::make_shared<VulkanImage>(desc));
	}
}

VulkanAttachment::VulkanAttachment(std::vector<VkImage>& imageVec, std::vector<VulkanImageView::ImageViewPtr>& imageViewVec)
{
	assert(imageVec.size() == gSwapChainImageCount);
	mImages.resize(gSwapChainImageCount);	
	for (int i = 0 ; i < imageVec.size(); i ++)
	{
		mImages[i] = std::make_shared<VulkanImage>(imageVec[i], imageViewVec[i]);
	}
}

VulkanAttachment::~VulkanAttachment()
{
}

void VulkanAttachment::TranslateAttachmentImage(int frameIndex, VkCommandBuffer commandBuffer)
{
	if (mIsColor)
	{
		std::dynamic_pointer_cast<VulkanImage>(mImages[frameIndex])->TranslateImageLayout(commandBuffer, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}
