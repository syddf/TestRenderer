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

void VulkanAttachment::TranslateAttachmentImage(int frameIndex, VkImageLayout layout)
{
	if(mIsColor)
		std::dynamic_pointer_cast<VulkanImage>(mImages[frameIndex])->SetImageLayout(layout);
}
