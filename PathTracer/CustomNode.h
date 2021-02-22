#pragma once

#include "Vulkan/ComputePass.h"
#include "Vulkan/RenderPass.h"

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


class BlurCustomNode : public VulkanRenderingCustomNode
{
public:
	BlurCustomNode(VulkanMaterial::MaterialPtr material, World* world, std::vector<IImage::ImagePtr> srcImage, std::vector<IImage::ImagePtr> blurImage, Vec2 blurDirection);
	virtual void RecordCommandBuffer(VkCommandBuffer& commandBuffer, int frameIndex, VkPipelineLayout pipelineLayout);
	void CreateResources();

private:
	void CreateQuadMesh();

	VulkanMaterial::MaterialPtr mMaterial;
	World* mWorld;
	std::vector<IImage::ImagePtr> mSrcImage;
	std::vector<IImage::ImagePtr> mBlurImage;

	WorldMaterialResource mCameraMaterialResource;
	Vec2 mBlurDirection;
};