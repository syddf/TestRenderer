#pragma once
#include "VulkanCommon.h"
#include "ImageView.h"
#include "VulkanAttachment.h"

class VulkanSwapChain : public ISwapChain
{
public:
	VulkanSwapChain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, UInt32 graphicsFamily, UInt32 presentFamily);
	~VulkanSwapChain();

public:
	using ImageViewPtr = VulkanImageView::ImageViewPtr;

public:
	void InitializeSwapChain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, UInt32 graphicsFamily, UInt32 presentFamily);

	VkSwapchainKHR GetSwapChainKHR () const { return mSwapChain; };

private:
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VulkanAttachment::AttachmentPtr mSwapChainImageAttachment;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	VkDevice mDevice;
	UInt32 mSwapChainImageCount;

	std::vector<ImageViewPtr> mSwapChainImageViews;
};