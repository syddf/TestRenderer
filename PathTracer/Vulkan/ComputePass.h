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

class ComputeNode
{
public:
	ComputeNode(ComputeNodeDesc desc);
	~ComputeNode();

private:
	void CreateComputePipeline(const ComputeNodeDesc& desc);
	void GenerateCommandBuffer();
	VkCommandBuffer RecordCommandBuffer(int frameIndex);

private:
	VkPipelineLayout mPipelineLayout;
	VkPipeline mComputePipeline;
	VulkanMaterial* mMaterial;
	World* mWorld;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<bool> mDirty;
	Vec3 mInvocation;
};