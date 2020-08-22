#include "SwapChain.h"

extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;
UInt32 gSwapChainImageCount = 0;

VulkanSwapChain::VulkanSwapChain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, UInt32 graphicsFamily, UInt32 presentFamily)
{
	InitializeSwapChain(physicalDevice, surface, device, graphicsFamily, presentFamily);
	mDevice = device;
}

VulkanSwapChain::~VulkanSwapChain()
{
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

void VulkanSwapChain::InitializeSwapChain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, UInt32 graphicsFamily, UInt32 presentFamily)
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	UInt32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	}

	UInt32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
	}

	UInt32 formatIndex = 0;
	for (UInt32 i = 0; i < formatCount; i ++)
	{
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			formatIndex = i;
			break;
		}
	}

	UInt32 presentModeIndex = 0;
	for (UInt32 i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			presentModeIndex = i;
			break;
		}
	}


	VkExtent2D actualExtent = { gScreenWidth, gScreenHeight };
	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

	if (capabilities.minImageCount <= 3 && capabilities.maxImageCount >= 3)
	{
		mSwapChainImageCount = 3;
	}
	else if (capabilities.minImageCount <= 2 && capabilities.maxImageCount >= 2)
	{
		mSwapChainImageCount = 3;
	}

	uint32_t imageCount = mSwapChainImageCount;
	gSwapChainImageCount = mSwapChainImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = formats[formatIndex].format;
	createInfo.imageColorSpace = formats[formatIndex].colorSpace;
	createInfo.imageExtent = actualExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	UInt32 queueFamilyIndices[] = { graphicsFamily, presentFamily };
	if (graphicsFamily != presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentModes[presentModeIndex];
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VKFUNC(vkCreateSwapchainKHR(device, &createInfo, nullptr, &mSwapChain), "Failed To Create SwapChain");
	vkGetSwapchainImagesKHR(device, mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, mSwapChain, &imageCount, mSwapChainImages.data());

	mSwapChainImageFormat = formats[formatIndex].format;
	mSwapChainExtent = actualExtent;

	for (size_t imageIndex = 0; imageIndex < mSwapChainImages.size(); imageIndex++)
	{
		ImageViewDesc desc;
		desc.BaseArrayLayer = 0;
		desc.BaseMipLevel = 0;
		desc.LayerCount = 1;
		desc.MipLevelCount = 1;
		desc.Dimension = TextureDimension::Texture2D;
		desc.Format = TextureFormat::TF_B8G8R8A8SRGB;
		desc.ImageHandleAddr = reinterpret_cast<char*>(&mSwapChainImages[imageIndex]);
		desc.IsColorTexture = true;

		mSwapChainImageViews.push_back(std::make_shared<VulkanImageView>(desc));
	}
}
