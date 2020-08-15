#pragma once
#include "VulkanCommon.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanWindow : public IWindow
{
public:
	VulkanWindow(bool bMainShowWindow, VkInstance instance);
	~VulkanWindow();

public:
	void CreateGLFWWindow();
	void InitializeSurface(VkInstance instance);
	void MainLoop();
	bool InitPresentFamily(VkPhysicalDevice physicalDevice);

	VkSurfaceKHR GetVulkanSurface() const { return mSurface;  }
	UInt32 GetPresentFamily() const { return mPresentFamily; }

private:
	VkSurfaceKHR mSurface;
	GLFWwindow* mWindow;
	VkInstance mInstance;
	UInt32 mPresentFamily;
};
