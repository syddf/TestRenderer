#pragma once
#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"

class VulkanImageView : public IImageView
{
public:
	VulkanImageView(ImageViewDesc desc);
	VulkanImageView(VkImageView imageView);
	~VulkanImageView();

public:
	using ImageViewPtr = std::shared_ptr<VulkanImageView>;

public:
	void CreateImageView(ImageViewDesc desc);
	char* GetGPUImageViewHandleAddress();

private:
	VkImageView mImageView;
};