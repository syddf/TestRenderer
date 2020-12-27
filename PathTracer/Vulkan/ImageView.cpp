#include "ImageView.h"
#include "Device.h"

extern VkDevice gVulkanDevice;

VulkanImageView::VulkanImageView(ImageViewDesc desc)
{
	CreateImageView(desc);
}

VulkanImageView::VulkanImageView(VkImageView imageView)
{
	mImageView = imageView;
}

VulkanImageView::~VulkanImageView()
{
	vkDestroyImageView(gVulkanDevice, mImageView, nullptr);
}

void VulkanImageView::CreateImageView(ImageViewDesc desc)
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.format = GetVKTextureFormat(desc.Format);
	imageViewCreateInfo.viewType = GetVKTextureViewType(desc.Dimension);
	imageViewCreateInfo.subresourceRange.aspectMask = desc.IsColorTexture ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = desc.BaseArrayLayer;
	imageViewCreateInfo.subresourceRange.layerCount = desc.LayerCount;
	imageViewCreateInfo.subresourceRange.baseMipLevel = desc.BaseMipLevel;
	imageViewCreateInfo.subresourceRange.levelCount = desc.MipLevelCount;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.image = *reinterpret_cast<VkImage*>(desc.ImageHandleAddr);

	VKFUNC(vkCreateImageView(gVulkanDevice, &imageViewCreateInfo, nullptr, &mImageView), "Create Image View Failed.");
}

char* VulkanImageView::GetGPUImageViewHandleAddress()
{
	return reinterpret_cast<char*>(&mImageView);
}
