#pragma once
#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "VulkanAttachment.h"
#include "./../WorldCommon/WorldObject.h"
#include "./../WorldCommon/World.h"

struct RenderingNodeDesc
{
	char* MaterialAddr;
	std::vector<WorldObject::ObjectPtr> Object;
	int EmptyVertexCount;
	World* World;
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
	World* mWorld;
	std::vector<WorldObject::ObjectPtr> mObject;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<bool> mDirty;
	int mEmptyVertexCount;
};

class VulkanPipelineNode : public IRenderingPipelineNode
{
public:
	VulkanPipelineNode(const RenderingPipelineNodeDesc & desc);
	~VulkanPipelineNode();
	using RenderingNodePtr = std::shared_ptr<VulkanRenderingNode>;

public:
	void GenerateGraphicsNode(const RenderingPipelineNodeDesc& desc);
	VkSemaphore& GetSignalSemaphore(int frameIndex) { return mSignalSemaphore[frameIndex]; }
	std::vector<VkSemaphore>& GetWaitSemaphore(int frameIndex) { return mWaitSemaphore[frameIndex]; }
	std::vector<VkPipelineStageFlags>& GetWaitFlags() { mWaitFlags.resize(mWaitSemaphore[0].size(), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT); return mWaitFlags; }
	void AddWaitSemaphore(VkSemaphore waitSemaphore, int frameIndex) { assert(waitSemaphore != VK_NULL_HANDLE); mWaitSemaphore[frameIndex].push_back(waitSemaphore); };
	void CreateSignalSemaphore();
	void AddRenderingNodes(RenderingNodeDesc desc);
	VkCommandBuffer& RecordCommandBuffer(int frameIndex);

private:
	VkRenderPass mVKRenderPass;
	std::vector<VulkanAttachment::AttachmentPtr> mAttachment;
	std::vector<VkFramebuffer> mFrameBuffer;
	std::vector<VkSemaphore> mSignalSemaphore;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<RenderingNodePtr> mRenderingNodes;
	std::vector<std::vector<VkSemaphore>> mWaitSemaphore;
	std::vector<VkPipelineStageFlags> mWaitFlags;
	std::vector<VkClearValue> mClearValue;
	int mColorAttachmentCount;
};
