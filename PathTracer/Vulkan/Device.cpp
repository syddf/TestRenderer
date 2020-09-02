#include "Device.h"
#include "TranslateEngine.h"
#include "CommandBufferPool.h"
#include <GLFW/glfw3.h>

extern bool gNeedDebugLayer;
bool enableValidationLayers = true;
VkDevice gVulkanDevice = VK_NULL_HANDLE;
VkPhysicalDevice gVulkanPhysicalDevice = VK_NULL_HANDLE;
ITranslationEngine* gTranslateEngine = nullptr;
VulkanCommandBufferPool* gGraphicsCommonCommandBufferPool = nullptr;
VulkanCommandBufferPool* gPipelineGraphicsSecondaryCommandBufferPool = nullptr;
VulkanCommandBufferPool* gPipelineGraphicsPrimaryCommandBufferPool = nullptr;

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
	std::string message = std::string("Validation layer: ") + std::string(pCallbackData->pMessage);
	OutputInformationHelper::OutputErrorInfomation(message);
	return VK_FALSE;
}

VulkanDevice::VulkanDevice()
{
	InitializeInstance();
	InitializePhysicalDevice();
	InitializeLogicalDevice();
}

VulkanDevice::~VulkanDevice()
{
	delete gTranslateEngine;
	delete gGraphicsCommonCommandBufferPool;
	delete gPipelineGraphicsSecondaryCommandBufferPool;

	vkDestroyDevice(mDevice, nullptr);

	if (enableValidationLayers && gNeedDebugLayer) 
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) 
		{
			func(mInstance, mDebugMessenger, nullptr);
		}
	}

	vkDestroyInstance(mInstance, nullptr);
}

void VulkanDevice::InitializeInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Render";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	UInt32 glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	if (gNeedDebugLayer)
	{
		UInt32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto& validationName : validationLayers)
		{
			bool enableValidation = false;
			for (auto& layerProp : availableLayers)
			{
				if (strcmp(layerProp.layerName,validationName) == 0)
				{
					enableValidation = true;
				}
			}
			if (!enableValidation)
			{
				OutputInformationHelper::OutputErrorInfomation(std::string("Not support validation layer : ") + validationName);
				enableValidationLayers = false;
				break;
			}
		}

		if (enableValidationLayers)
		{
			instanceCreateInfo.enabledLayerCount = validationLayers.size();
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instanceCreateInfo.enabledExtensionCount = extensions.size();
			instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
		}
	}

	VKFUNC(vkCreateInstance(&instanceCreateInfo, NULL, &mInstance), "Create Vulkan Instance Failed.");
	
	if (gNeedDebugLayer && enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
		{
			OutputInformationHelper::OutputErrorInfomation("Create Debug Messenger Failed.");
		}
	}
}

void VulkanDevice::InitializePhysicalDevice()
{
	mTransferQueueFamily = -1;
	mGraphicsQueueFamily = -1;
	mComputeQueueFamily = -1;

	UInt32 deviceCount;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());
	if (deviceCount == 0)
	{
		OutputInformationHelper::OutputErrorInfomation("No Physical Device.");
		throw 0;
	}

	UInt32 physicalDeviceIndex = 0;
	for (; physicalDeviceIndex < deviceCount; physicalDeviceIndex++)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(devices[physicalDeviceIndex], &deviceProperties);
		vkGetPhysicalDeviceFeatures(devices[physicalDeviceIndex], &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			mPhysicalDevice = devices[physicalDeviceIndex];
			gVulkanPhysicalDevice = mPhysicalDevice;
			break;
		}
	}

	if (physicalDeviceIndex == deviceCount)
	{
		OutputInformationHelper::OutputErrorInfomation("No Physical Device.");
		throw 0;
	}
	UInt32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilies.data());

	UInt32 queueIndex = 0;
	for (const auto& queueFamily : queueFamilies) 
	{
		if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && mGraphicsQueueFamily == -1) 
		{
			mGraphicsQueueFamily = queueIndex;
		}

		if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && mComputeQueueFamily == -1)
		{
			mComputeQueueFamily = queueIndex;
		}

		if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && mTransferQueueFamily == -1)
		{
			mTransferQueueFamily = queueIndex;
		}
		queueIndex++;
	}
}

void VulkanDevice::InitializeLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVec;
	if (mGraphicsQueueFamily != -1)
	{
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.queueFamilyIndex = mGraphicsQueueFamily;
		float queuePriority = 1.0f;
		graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfoVec.push_back(graphicsQueueCreateInfo);
	}

	if (mComputeQueueFamily != -1 && mComputeQueueFamily != mGraphicsQueueFamily)
	{
		VkDeviceQueueCreateInfo computeQueueCreateInfo = {};
		computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		computeQueueCreateInfo.queueCount = 1;
		computeQueueCreateInfo.queueFamilyIndex = mComputeQueueFamily;
		float queuePriority = 1.0f;
		computeQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfoVec.push_back(computeQueueCreateInfo);
	}

	if (mTransferQueueFamily != -1 && mTransferQueueFamily != mGraphicsQueueFamily && mTransferQueueFamily != mGraphicsQueueFamily)
	{
		VkDeviceQueueCreateInfo transferQueueCreateInfo = {};
		transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		transferQueueCreateInfo.queueCount = 1;
		transferQueueCreateInfo.queueFamilyIndex = mTransferQueueFamily;
		float queuePriority = 1.0f;
		transferQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfoVec.push_back(transferQueueCreateInfo);
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfoVec.data();
	createInfo.queueCreateInfoCount = queueCreateInfoVec.size();
	VkPhysicalDeviceFeatures deviceFeatures{};
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;
	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	if (enableValidationLayers && gNeedDebugLayer) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}
	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	VKFUNC(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice), "Failed To Create Logical Device.");
	vkGetDeviceQueue(mDevice, mGraphicsQueueFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, mComputeQueueFamily, 0, &mComputeQueue);
	vkGetDeviceQueue(mDevice, mTransferQueueFamily, 0, &mTransferQueue);

	if (gVulkanDevice == VK_NULL_HANDLE)
	{
		gVulkanDevice = mDevice;
	}

	gTranslateEngine = new VulkanTranslateEngine(mTransferQueueFamily, mTransferQueue);
	gGraphicsCommonCommandBufferPool = new VulkanCommandBufferPool(mGraphicsQueueFamily, true, false, mGraphicsQueue);
	gPipelineGraphicsSecondaryCommandBufferPool = new VulkanCommandBufferPool(mGraphicsQueueFamily, false, true, mGraphicsQueue);
	gPipelineGraphicsPrimaryCommandBufferPool = new VulkanCommandBufferPool(mGraphicsQueueFamily, false, true, mGraphicsQueue);
}
