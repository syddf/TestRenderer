#include "ComputePass.h"
#include "CommandBufferPool.h"

extern VkDevice gVulkanDevice;
extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;
extern UInt32 gSwapChainImageCount;
extern VulkanCommandBufferPool* gPipelineGraphicsPrimaryCommandBufferPool;

VulkanComputeNode::VulkanComputeNode(ComputeNodeDesc desc)
{
	mMaterial = reinterpret_cast<VulkanMaterial*>(desc.MaterialAddr);
	mWorld = desc.World;
	mInvocation = desc.Invocation;

	CreateComputePipeline(desc);
}

VulkanComputeNode::~VulkanComputeNode()
{
}

void VulkanComputeNode::CreateComputePipeline(const ComputeNodeDesc& desc)
{
	auto& shaderStageInfo = mMaterial->GetShaderStageInfo();
	assert(shaderStageInfo.size() == 1);
	auto& descSetVec = mMaterial->GetSetLayouts();
	auto& pushConstantRangeVec = mMaterial->GetPushConstantRange();

	VkPipelineLayoutCreateInfo plCreateInfo = {};
	plCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plCreateInfo.pSetLayouts = descSetVec.data();
	plCreateInfo.setLayoutCount = descSetVec.size();
	plCreateInfo.pPushConstantRanges = pushConstantRangeVec.data();
	plCreateInfo.pushConstantRangeCount = pushConstantRangeVec.size();
	plCreateInfo.pPushConstantRanges = pushConstantRangeVec.data();
	VKFUNC(vkCreatePipelineLayout(gVulkanDevice, &plCreateInfo, nullptr, &mPipelineLayout), "Create Pipeline Layout Failed.");

	VkComputePipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stage = shaderStageInfo[0];
	pipelineCreateInfo.layout = mPipelineLayout;
	VKFUNC(vkCreateComputePipelines(gVulkanDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mComputePipeline), "Create Pipeline Failed.");
}

void VulkanComputeNode::RecordCommandBuffer(VkCommandBuffer& commandBuffer, int frameIndex)
{
	mMaterial->Update(frameIndex);
	Vec3 localSize = mMaterial->GetComputeLocalSize();
	int groupSizeX = std::ceilf(mInvocation.x / localSize.x);
	int groupSizeY = std::ceilf(mInvocation.y / localSize.y);
	int groupSizeZ = std::ceilf(mInvocation.z / localSize.z);
	int descCount;
	std::vector<VkDescriptorSet> submitDescSetVec =mMaterial->GetDescriptorSet(frameIndex, descCount, true);
	VkDescriptorSet worldSet = mWorld->GetWorldParamsSet(mMaterial, frameIndex);
	if (worldSet != VK_NULL_HANDLE)
		submitDescSetVec.push_back(worldSet);
	vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, submitDescSetVec.size(), submitDescSetVec.data(), 0, nullptr);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);
	vkCmdDispatch(commandBuffer, groupSizeX, groupSizeY, groupSizeZ);
}
