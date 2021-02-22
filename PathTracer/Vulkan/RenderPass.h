#pragma once
#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "VulkanAttachment.h"
#include "./../WorldCommon/WorldObject.h"
#include "./../WorldCommon/World.h"
#include "ComputePass.h"

class VulkanRenderingCustomNode
{
public:
	using CustomNodePtr = std::shared_ptr<VulkanRenderingCustomNode>;
	virtual void RecordCommandBuffer(VkCommandBuffer& commandBuffer, int frameIndex, VkPipelineLayout pipelineLayout) = 0;
};

struct RenderingNodeDesc
{
	char* MaterialAddr;
	std::vector<WorldObject::ObjectPtr> Object;
	int EmptyVertexCount;
	World* World;
	VulkanRenderingCustomNode::CustomNodePtr CustomNode = nullptr;
};

struct RenderingPipelineNodeDesc
{
	FrameBufferLayoutDesc FrameBufferLayoutDesc;
	FrameBufferDesc FrameBufferDesc;
	std::vector<std::string> DependingAttachmentViewName;
	std::vector<RenderingNodeDesc> RenderingNodeDescVec;
	std::vector<ComputeNodeDesc> ComputeNodeDescVec;
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
	VulkanRenderingNode(RenderingNodeDesc desc, VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo blendState, int width, int height);
	~VulkanRenderingNode();

public:
	void CreatePipeline(RenderingNodeDesc desc, VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo blendState, int width, int height);
	void GenerateCommandBuffer();
	VkCommandBuffer RecordCommandBuffer(int frameIndex, VkRenderPassBeginInfo renderPassInfo);

private:
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;
	char* mMaterialAddr;
	World* mWorld;
	std::vector<WorldObject::ObjectPtr> mObject;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<bool> mDirty;
	int mEmptyVertexCount;
	VulkanRenderingCustomNode::CustomNodePtr mCustomNodePtr;
};

class VulkanPipelineNode : public IRenderingPipelineNode
{
public:
	VulkanPipelineNode(const RenderingPipelineNodeDesc & desc);
	~VulkanPipelineNode();
	using RenderingNodePtr = std::shared_ptr<VulkanRenderingNode>;
	using ComputeNodePtr = std::shared_ptr<VulkanComputeNode>;

public:
	void GenerateGraphicsNode(const RenderingPipelineNodeDesc& desc);
	VkSemaphore& GetSignalSemaphore(int frameIndex) { return mSignalSemaphore[frameIndex]; }
	std::vector<VkSemaphore>& GetWaitSemaphore(int frameIndex) { return mWaitSemaphore[frameIndex]; }
	std::vector<VkPipelineStageFlags>& GetWaitFlags() 
	{ 
		if(!mIsComputeNode)
			mWaitFlags.resize(mWaitSemaphore[0].size(), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		else
			mWaitFlags.resize(mWaitSemaphore[0].size(), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		return mWaitFlags; 
	}
	void AddWaitSemaphore(VkSemaphore waitSemaphore, int frameIndex) { assert(waitSemaphore != VK_NULL_HANDLE); mWaitSemaphore[frameIndex].push_back(waitSemaphore); };
	void CreateSignalSemaphore();
	void AddRenderingNodes(RenderingNodeDesc desc);
	void AddComputeNodes(ComputeNodeDesc desc);
	VkCommandBuffer& RecordCommandBuffer(int frameIndex);

private:
	VkRenderPass mVKRenderPass;
	std::vector<VulkanAttachment::AttachmentPtr> mAttachment;
	std::vector<VkFramebuffer> mFrameBuffer;
	std::vector<VkSemaphore> mSignalSemaphore;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<RenderingNodePtr> mRenderingNodes;
	std::vector<ComputeNodePtr> mComputeNodes;
	
	std::vector<std::vector<VkSemaphore>> mWaitSemaphore;
	std::vector<VkPipelineStageFlags> mWaitFlags;
	std::vector<VkClearValue> mClearValue;
	int mColorAttachmentCount;
	int mAttachmentWidth;
	int mAttachmentHeight;
	bool mIsComputeNode;
	bool mNeedPresent;
};
