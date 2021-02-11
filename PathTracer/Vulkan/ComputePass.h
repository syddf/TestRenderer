#pragma once

#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "VulkanAttachment.h"
#include "./../WorldCommon/WorldObject.h"
#include "./../WorldCommon/World.h"

struct ComputeNodeDesc
{
	char* MaterialAddr;
	World* World;
	Vec3 Invocation;
};

class VulkanComputeNode
{
public:
	VulkanComputeNode(ComputeNodeDesc desc);
	~VulkanComputeNode();

private:
	void CreateComputePipeline(const ComputeNodeDesc& desc);

public:
	void RecordCommandBuffer(VkCommandBuffer& commandBuffer, int frameIndex);

private:
	VkPipelineLayout mPipelineLayout;
	VkPipeline mComputePipeline;
	VulkanMaterial* mMaterial;
	World* mWorld;
	Vec3 mInvocation;
};