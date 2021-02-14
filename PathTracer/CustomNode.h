#pragma once

#include "Vulkan/ComputePass.h"

class VoxelMipMapCustomNode : public VulkanComputeNode
{
public:
	VoxelMipMapCustomNode(ComputeNodeDesc desc);
	~VoxelMipMapCustomNode();

public:
	virtual void RecordCommandBuffer(VkCommandBuffer& commandBuffer, int frameIndex);


private:
	void CreateMipMapDescriptorSet();

	std::vector<WorldMaterialResource> mCameraMaterialResource;
};