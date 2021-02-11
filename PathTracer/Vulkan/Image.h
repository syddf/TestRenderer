#pragma once
#include "VulkanCommon.h"
#include "ImageView.h"

class VulkanImage : public IImage
{
public:
	VulkanImage(ImageDesc desc);
	VulkanImage(VkImage image, VulkanImageView::ImageViewPtr imageView);
	~VulkanImage();

public:
	void CreateImage(ImageDesc desc);
	void TranslateImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkImageAspectFlags aspect, UInt32 mipLevels);
	void TranslateImageLayout(VkImageLayout newLayout, VkImageAspectFlags aspect, UInt32 mipLevels);
	void GenerateMipMap(UInt32 Width, UInt32 Height, UInt32 MipLevels);
	void AddImageView(std::string formatName);
	char* GetGPUImageHandleAddress();
	char* GetGPUImageViewHandleAddress();
	char* GetGPUImageViewHandleAddress(std::string formatName);
	char* GetSamplerHandleAddress();

private:
	VkImage mImage;
	VkDeviceMemory mImageMemory;
	VkSampler mSampler;
	VkImageLayout mImageLayout;
	IImageView::ImageViewPtr mImageView;
	ImageViewDesc mImageViewDesc;
	std::map<std::string, IImageView::ImageViewPtr> mOtherFormatImageViewMap;
};