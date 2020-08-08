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

void VulkanWindow::MainLoop()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
	}
}
