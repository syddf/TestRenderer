#pragma once
#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "VulkanAttachment.h"
#include "./../WorldCommon/WorldObject.h"

struct RenderingNodeDesc
{
	char* MaterialAddr;
	std::vector<WorldObject::ObjectPtr> Object;
};

struct RenderingPipelineNodeDesc
{
	FrameBufferLayoutDesc FrameBufferLayoutDesc;
	FrameBufferDesc FrameBufferDesc;
	std::vector<std::string> DependingAttachmentViewName;
	std::vector<RenderingNodeDesc> RenderingNodeDescVec;
	PipelineBindPoint BindPoint;
	std::string NodeName;
	bool AttachToWindowNode;

	//..
	std::vector<int> DependingNodeIndex;
	bool AffectOtherNode;
};


class VulkanRenderingNode
{
public:
	VulkanRenderingNode(RenderingNodeDesc desc, VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo blendState);
	~VulkanRenderingNode();

public:
	void CreatePipeline(RenderingNodeDesc desc, VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo blendState);
	void GenerateCommandBuffer();
	VkCommandBuffer RecordCommandBuffer(int frameIndex, VkRenderPassBeginInfo renderPassInfo);

private:
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;
	char* mMaterialAddr;
	std::vector<WorldObject::ObjectPtr> mObject;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<bool> mDirty;
};

class VulkanPipelineNode : public IRenderingPipelineNode
{
public:
	VulkanPipelineNode(const RenderingPipelineNodeDesc & desc);
	~VulkanPipelineNode();
	using RenderingNodePtr = std::shared_ptr<VulkanRenderingNode>;

public:
	void GenerateGraphicsNode(const RenderingPipelineNodeDesc& desc);
	VkSemaphore GetSignalSemaphore(int frameIndex) const { return mSignalSemaphore[frameIndex]; }
	std::vector<VkSemaphore>& GetWaitSemaphore(int frameIndex) { return mWaitSemaphore[frameIndex]; }
	void AddWaitSemaphore(VkSemaphore waitSemaphore, int frameIndex) { assert(waitSemaphore != VK_NULL_HANDLE); mWaitSemaphore[frameIndex].push_back(waitSemaphore); };
	void CreateSignalSemaphore();
	void AddRenderingNodes(RenderingNodeDesc desc);
	VkCommandBuffer RecordCommandBuffer(int frameIndex);

private:
	VkRenderPass mVKRenderPass;
	std::vector<VulkanAttachment::AttachmentPtr> mAttachment;
	std::vector<VkFramebuffer> mFrameBuffer;
	std::vector<VkSemaphore> mSignalSemaphore;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<RenderingNodePtr> mRenderingNodes;
	std::vector<std::vector<VkSemaphore>> mWaitSemaphore;
	std::vector<VkClearValue> mClearValue;
	int mColorAttachmentCount;
};
