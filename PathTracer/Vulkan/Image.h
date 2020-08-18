#pragma once
#include "VulkanCommon.h"

class VulkanImage : public IImage
{
public:
	VulkanImage(ImageDesc desc);
	~VulkanImage();

public:
	void CreateImage(ImageDesc desc);
	void TranslateImageLayout(VkImageLayout newLayout, VkImageAspectFlags aspect, UInt32 mipLevels);
	void GenerateMipMap(UInt32 Width, UInt32 Height, UInt32 MipLevels);
	char* GetGPUImageHandleAddress();
	char* GetGPUImageViewHandleAddress();

private:
	VkImage mImage;
	VkDeviceMemory mImageMemory;
	VkImageLayout mImageLayout;
	IImageView::ImageViewPtr mImageView;
};