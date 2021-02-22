#pragma once
#include "VulkanCommon.h"
#include "Image.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "ImageView.h"

class VulkanAttachment
{
public:
	VulkanAttachment(ImageDesc desc, bool bIsColor);
	VulkanAttachment(std::vector<VkImage>& imageVec, std::vector<VulkanImageView::ImageViewPtr>& imageViewVec);
	~VulkanAttachment();
	using AttachmentPtr = std::shared_ptr<VulkanAttachment>;

public:
	std::shared_ptr<VulkanImage> GetImage(int frameIndex) { return std::dynamic_pointer_cast<VulkanImage>(mImages[frameIndex]); };
	void TranslateAttachmentImage(int frameIndex, VkImageLayout layout);

private:
	std::vector<VulkanImage::ImagePtr> mImages;
	bool mIsColor;
};