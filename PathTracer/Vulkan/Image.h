#pragma once
#include "VulkanCommon.h"

class VulkanImage : public IImage
{
public:
	VulkanImage(ImageDesc desc);
	~VulkanImage();

public:
	void CreateImage(ImageDesc desc);

private:
	VkImage mImage;
	VkDeviceMemory mImageMemory;
};