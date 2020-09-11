#pragma once

#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"

class VulkanPresentEngine
{
public:
	VulkanPresentEngine(VkSwapchainKHR swapChain, VkQueue presentQueue);
	~VulkanPresentEngine();

public:
	int AcquireImage();
	void PresentFrame(int frameIndex, std::vector<VkSemaphore>& renderFinishSemaphore);

private:
	int mCurrentFrame;
	VkQueue mPresentQueue;
	VkSwapchainKHR mSwapChain;
	std::array<VkSemaphore, 3> mAcquireImageSemaphore;
	std::array<VkSemaphore, 3> mRenderFinishSemaphore;
	std::array<VkFence, 3> mWaitFence;
};