#pragma once
#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"

class VulkanRenderingNode : public IRenderingPipelineNode
{
public:
	VulkanRenderingNode(const RenderingPipelineNodeDesc & desc);
	~VulkanRenderingNode();

public:
	void GenerateGraphicsNode(const RenderingPipelineNodeDesc& desc);
	VkSemaphore GetSignalSemaphore() const { return mSignalSemaphore; }
	void AddWaitSemaphore(VkSemaphore waitSemaphore) { assert(waitSemaphore != VK_NULL_HANDLE); mWaitSemaphore.push_back(waitSemaphore); };
	void CreateSignalSemaphore();

private:
	VkRenderPass mVKRenderPass;
	VkFramebuffer mFrameBuffer;

	 VkSemaphore mSignalSemaphore;
	 std::vector<VkSemaphore> mWaitSemaphore;
};