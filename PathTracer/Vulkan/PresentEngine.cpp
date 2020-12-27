#include "PresentEngine.h"

extern VkDevice gVulkanDevice;


VulkanPresentEngine::VulkanPresentEngine(VkSwapchainKHR swapChain, VkQueue presentQueue)
{
	mSwapChain = swapChain;
	mPresentQueue = presentQueue;
	mCurrentFrame = 0;
	for (int i = 0; i < 3; i++)
	{
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		VKFUNC(vkCreateFence(gVulkanDevice, &fenceInfo, nullptr, &mWaitFence[i]), "Create Fence Failed.");
		fenceInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
		VKFUNC(vkCreateFence(gVulkanDevice, &fenceInfo, nullptr, &mRenderFinishFence[i]), "Create Fence Failed.");

		VkSemaphoreCreateInfo semaInfo = {};
		semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VKFUNC(vkCreateSemaphore(gVulkanDevice, &semaInfo, nullptr, &mAcquireImageSemaphore[i]), "Create Semaphore Failed.");
		VKFUNC(vkCreateSemaphore(gVulkanDevice, &semaInfo, nullptr, &mRenderFinishSemaphore[i]), "Create Semaphore Failed.");
	}
}

VulkanPresentEngine::~VulkanPresentEngine()
{
	for (int i = 0; i < 3; i++)
	{
		vkDestroySemaphore(gVulkanDevice, mAcquireImageSemaphore[i], nullptr);
		vkDestroySemaphore(gVulkanDevice, mRenderFinishSemaphore[i], nullptr);
		vkDestroyFence(gVulkanDevice, mWaitFence[i], nullptr);
	}
}

int VulkanPresentEngine::AcquireImage()
{
	UInt32 imageIndex;
	vkWaitForFences(gVulkanDevice, 1, &mRenderFinishFence[mCurrentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(gVulkanDevice, 1, &mRenderFinishFence[mCurrentFrame]);
	vkAcquireNextImageKHR(gVulkanDevice, mSwapChain, UINT64_MAX, mAcquireImageSemaphore[mCurrentFrame], mWaitFence[mCurrentFrame], &imageIndex);
	vkWaitForFences(gVulkanDevice, 1, &mWaitFence[mCurrentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(gVulkanDevice, 1, &mWaitFence[mCurrentFrame]);
	return imageIndex;
}

void VulkanPresentEngine::PresentFrame(int frameIndex, std::vector<VkSemaphore>& renderFinishSemaphore)
{
	static bool FirstPresent = true;
	UInt32 fInd = frameIndex;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapChain;
	presentInfo.waitSemaphoreCount = renderFinishSemaphore.size();
	presentInfo.pWaitSemaphores = renderFinishSemaphore.data();
	presentInfo.pImageIndices = &fInd;
	if (FirstPresent)
	{
		vkQueuePresentKHR(mPresentQueue, &presentInfo);
		if (mCurrentFrame == 1)
		{
			FirstPresent = false;
		}
	}
	else
	{
		// 2 frame before has been acquired. 1 frame before has been present.
		int nextFrame = (mCurrentFrame + 1) % 3;
		std::vector<VkSemaphore> waitSemaphoreVec = renderFinishSemaphore;
		waitSemaphoreVec.push_back(mAcquireImageSemaphore[nextFrame]);
		presentInfo.waitSemaphoreCount = waitSemaphoreVec.size();
		presentInfo.pWaitSemaphores = waitSemaphoreVec.data();
		vkQueuePresentKHR(mPresentQueue, &presentInfo);
	}
	mCurrentFrame = (mCurrentFrame + 1) % 3;
}
