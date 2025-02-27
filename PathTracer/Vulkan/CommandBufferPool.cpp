#include "CommandBufferPool.h"

extern VkDevice gVulkanDevice;

VulkanCommandBufferPool::VulkanCommandBufferPool(UInt32 QueueIndex, bool Transient, bool BufferIndividual, VkQueue Queue)
{
	InitializeCommandBufferPool(QueueIndex, Transient, BufferIndividual);
	mQueue = Queue;
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
	for (auto commandBuffer : mCommandBufferSet)
	{
		vkFreeCommandBuffers(gVulkanDevice, mCommandPool, 1, &commandBuffer);
	}
	vkDestroyCommandPool(gVulkanDevice, mCommandPool, nullptr);
}

VkCommandBuffer VulkanCommandBufferPool::BeginSingleTimeCommandBuffer(bool Primary)
{
	VkCommandBuffer commandBuffer = AllocateCommandBuffer(Primary);

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	return commandBuffer;
}

void VulkanCommandBufferPool::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
	VKFUNC(vkEndCommandBuffer(commandBuffer), "Failed To End CommandBuffer.");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mQueue);

	auto iter = mCommandBufferSet.find(commandBuffer);
	assert(iter != mCommandBufferSet.end());
	mCommandBufferSet.erase(iter);
	vkFreeCommandBuffers(gVulkanDevice, mCommandPool, 1, &commandBuffer);
}

VkCommandBuffer VulkanCommandBufferPool::AllocateCommandBuffer(bool Primary)
{
	VkCommandBufferAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocateInfo.commandPool = mCommandPool;
	bufferAllocateInfo.level = Primary ? VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY : VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	bufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VKFUNC(vkAllocateCommandBuffers(gVulkanDevice, &bufferAllocateInfo, &commandBuffer), "Failed To Allocate Command Buffer.");
	mCommandBufferSet.insert(commandBuffer);
	return commandBuffer;
}

void VulkanCommandBufferPool::BeginCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VulkanCommandBufferPool::BeginSecondaryCommandBuffer(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo & renderPassBeginInfo)
{
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.framebuffer = renderPassBeginInfo.framebuffer;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.subpass = 0;
	inheritanceInfo.renderPass = renderPassBeginInfo.renderPass;

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

}

void VulkanCommandBufferPool::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
	VKFUNC(vkEndCommandBuffer(commandBuffer), "Failed To End CommandBuffer.");
}

void VulkanCommandBufferPool::InitializeCommandBufferPool(UInt32 QueueIndex, bool Transient, bool BufferIndividual)
{
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex = QueueIndex;
	if (Transient)
	{
		poolCreateInfo.flags |= VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	}
	if (BufferIndividual)
	{
		poolCreateInfo.flags |= VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	}
	VKFUNC(vkCreateCommandPool(gVulkanDevice, &poolCreateInfo, nullptr, &mCommandPool), "Create Command Pool Failed.");
}
