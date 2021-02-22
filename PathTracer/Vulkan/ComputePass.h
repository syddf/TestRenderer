#pragma once

#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "VulkanAttachment.h"
#include "./../WorldCommon/WorldObject.h"
#include "./../WorldCommon/World.h"

class VulkanComputeNode;

struct ComputeNodeDesc
{
	char* MaterialAddr;
	World* World;
	Vec3 Invocation;

	std::shared_ptr<VulkanComputeNode> ComputeNode = nullptr;
};

class VulkanComputeNode
{
public:
	VulkanComputeNode(ComputeNodeDesc desc);
	~VulkanComputeNode();

protected:
	void CreateComputePipeline(const ComputeNodeDesc& desc);

public:
	virtual void RecordCommandBuffer(VkCommandBuffer& commandBuffer, int frameIndex);

protected:
	VkPipelineLayout mPipelineLayout;
	VkPipeline mComputePipeline;
	VulkanMaterial* mMaterial;
	World* mWorld;
	Vec3 mInvocation;
};
