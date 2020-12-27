#pragma once
#include "VulkanCommon.h"

class VulkanDevice : public IDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

public:
	VkInstance GetInstance() const { return mInstance; }
	VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
	VkDevice GetDevice() const { return mDevice; }
	UInt32 GetGraphicsFamily() const { return mGraphicsQueueFamily; }
	UInt32 GetComputeFamily() const { return mComputeQueueFamily; }
	UInt32 GetTransferFamily() const { return mTransferQueueFamily; }

	VkQueue GetGraphicsQueue() const { return mGraphicsQueue; }
	void FreeCommandPool();

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