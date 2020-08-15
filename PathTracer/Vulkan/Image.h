#pragma once
#include "VulkanCommon.h"

class VulkanImage : public IImage
{
public:
	VulkanImage();
	~VulkanImage();

private:
	VkImage mImage;
};