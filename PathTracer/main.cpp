#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "Vulkan/Device.h"
#include "Vulkan/Window.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/ResourceCreator.h"

bool gNeedDebugLayer = true;
UInt32 gScreenWidth = 800;
UInt32 gScreenHeight = 600;

int main() 
{	
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	IDevice* device = new VulkanDevice();
	IWindow* window = new VulkanWindow(true, static_cast<VulkanDevice*>(device)->GetInstance());
	if (static_cast<VulkanWindow*>(window)->InitPresentFamily(static_cast<VulkanDevice*>(device)->GetPhysicalDevice()))
	{
		ISwapChain* swapChain = new VulkanSwapChain
		(
			static_cast<VulkanDevice*>(device)->GetPhysicalDevice(),
			static_cast<VulkanWindow*>(window)->GetVulkanSurface(),
			static_cast<VulkanDevice*>(device)->GetDevice(),
			static_cast<VulkanDevice*>(device)->GetGraphicsFamily(),
			static_cast<VulkanWindow*>(window)->GetPresentFamily()
		);
		ResourceCreator::CreateImageFromFile("./../../Asset/Dst/red.data");
		ResourceCreator::CreateMaterial("D:\\Compile\\shaderVert.data", "D:\\Compile\\shaderFrag.data");
		ResourceCreator::CreateMeshFromFile("./../../Asset/Dst/Cornell.data", "D:\\Compile\\shaderVert.data", 0);
		window->MainLoop();
		ResourceCreator::DestroyCachingResource();
		delete swapChain;
	}
	delete window;
	delete device;

	return 0;
}