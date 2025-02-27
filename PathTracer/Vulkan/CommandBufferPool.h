#pragma once 
#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"

class VulkanCommandBufferPool
{
public:
	VulkanCommandBufferPool(UInt32 QueueIndex, bool Transient, bool BufferIndividual, VkQueue Queue);
	~VulkanCommandBufferPool();
	using CommandPoolPtr = std::shared_ptr<VulkanCommandBufferPool>;

public:
	VkCommandBuffer BeginSingleTimeCommandBuffer(bool Primary);
	void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);

	VkCommandBuffer AllocateCommandBuffer(bool Primary);
	void BeginCommandBuffer(VkCommandBuffer commandBuffer);
	void BeginSecondaryCommandBuffer(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo& renderPassBeginInfo);
	void EndCommandBuffer(VkCommandBuffer commandBuffer);

private:
	void InitializeCommandBufferPool(UInt32 QueueIndex, bool Transient, bool BufferIndividual);

private:
	VkCommandPool mCommandPool;
	VkQueue mQueue;
	std::set<VkCommandBuffer> mCommandBufferSet;
};