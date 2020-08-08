#pragma once
#include "./../../Source/Prefix.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
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

private:
	VkSurfaceKHR mSurface;
	GLFWwindow* mWindow;
	VkInstance mInstance;
};
