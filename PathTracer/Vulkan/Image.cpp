#include "Image.h"

extern VkDevice gVulkanDevice;

VulkanImage::VulkanImage(ImageDesc desc)
{
	CreateImage(desc);
}

VulkanImage::~VulkanImage()
{
}

void VulkanImage::CreateImage(ImageDesc desc)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.format = GetVKTextureFormat(desc.Format);
	imageCreateInfo.imageType = GetVKTextureType(desc.Dimension);
	imageCreateInfo.usage = GetVKImageUsageFlags(desc.Usage);
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent.width = desc.Width;
	imageCreateInfo.extent.height = desc.Height;
	imageCreateInfo.extent.depth = desc.Depth;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.mipLevels = desc.MipLevels;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.arrayLayers = desc.ArrayLayers;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	VKFUNC(vkCreateImage(gVulkanDevice, &imageCreateInfo, nullptr, &mImage), "Create Image Failed.");
}