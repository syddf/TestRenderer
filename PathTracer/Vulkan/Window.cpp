#include "Window.h"

extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;

VulkanWindow::VulkanWindow(bool bMainShowWindow, VkInstance instance)
{
	if (bMainShowWindow)
	{
		CreateGLFWWindow();
	}
	InitializeSurface(instance);
	mInstance = instance;
}

VulkanWindow::~VulkanWindow()
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
}

void VulkanWindow::CreateGLFWWindow()
{
	mWindow = glfwCreateWindow(gScreenWidth, gScreenHeight, "Vulkan window", nullptr, nullptr);
}

void VulkanWindow::InitializeSurface(VkInstance instance)
{
	VKFUNC(glfwCreateWindowSurface(instance, mWindow, nullptr, &mSurface), "Failed To Create Window Surface.");
}

bool VulkanWindow::MainLoop()
{
	bool close = glfwWindowShouldClose(mWindow);
	if (!close)
	{
		glfwPollEvents();
	}
	return close;
}

bool VulkanWindow::InitPresentFamily(VkPhysicalDevice physicalDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	UInt32 graphicsFamily = 0;
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mSurface, &presentSupport);

		if (presentSupport)
		{
			mPresentFamily = i;
			return true;
		}
		i++;
	}
	return false;
}
