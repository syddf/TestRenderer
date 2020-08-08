#pragma once
#include "./../../Source/Prefix.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "VulkanCommon.h"


class VulkanDevice : public IDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

public:
	VkInstance GetInstance() const { return mInstance; }

private:
	void InitializeInstance();
	void InitializePhysicalDevice();
	void InitializeLogicalDevice();

private:
	VkInstance mInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;

	UInt32 mGraphicsQueueFamily;
	UInt32 mComputeQueueFamily;
	UInt32 mTransferQueueFamily;

	VkQueue mGraphicsQueue;
	VkQueue mComputeQueue;
	VkQueue mTransferQueue;
};