#include "RenderingPipeline.h"
#include "RenderPass.h"

extern UInt32 gSwapChainImageCount;

VulkanRenderingPipeline::VulkanRenderingPipeline()
{
}

VulkanRenderingPipeline::~VulkanRenderingPipeline()
{
}

void VulkanRenderingPipeline::GenerateRenderingGraph(std::vector<RenderingPipelineNodeDesc>& nodesVec)
{
	for (size_t i = 0; i < nodesVec.size(); i ++)
	{
		for (size_t j = 0; j < nodesVec[i].FrameBufferDesc.AttachmentName.size(); j ++)
		{
			std::string attachmentViewName = nodesVec[i].FrameBufferDesc.AttachmentName[j];
			mAttachmentAffectNodesMap[attachmentViewName].push_back(i);
		}
	}

	std::vector<std::vector<int>> graphEdgeMap;
	graphEdgeMap.resize(nodesVec.size(), std::vector<int>(nodesVec.size(), 0));
	for (size_t i = 0; i < nodesVec.size(); i ++)
	{
		for (size_t j = 0; j < nodesVec[i].DependingAttachmentViewName.size(); j ++)
		{
			std::string dependingImageViewName = nodesVec[i].DependingAttachmentViewName[j];
			if (mAttachmentAffectNodesMap.find(dependingImageViewName) == mAttachmentAffectNodesMap.end())
			{
				throw "Unexpected Depending Attachment.";
			}
			else
			{
				auto dependingNodeVec = mAttachmentAffectNodesMap[dependingImageViewName];
				for (auto dependingNode : dependingNodeVec)
				{
					graphEdgeMap[dependingNode][i] = 1;
					nodesVec[i].DependingNodeIndex.push_back(dependingNode);
					nodesVec[dependingNode].AffectOtherNode = true;
				}
			}
		}
	}

	TopologySort(nodesVec, graphEdgeMap);

	for (size_t i = 0; i < nodesVec.size(); i ++)
	{
		mRenderingNodesVec[i] = std::make_shared<VulkanPipelineNode>(nodesVec[i]);
	}

	for (size_t i = 0; i < nodesVec.size(); i++)
	{
		static_cast<VulkanPipelineNode*>(mRenderingNodesVec[i].get())->CreateSignalSemaphore();
		if (!nodesVec[i].AffectOtherNode)
		{
			for (int frameIndex = 0; frameIndex < gSwapChainImageCount; frameIndex++)
			{
				mRenderFinishSemaphore[frameIndex].push_back(static_cast<VulkanPipelineNode*>(mRenderingNodesVec[i].get())->GetSignalSemaphore(frameIndex));
			}
		}
		auto dependingNodeIndexVec = nodesVec[i].DependingNodeIndex;
		for (size_t j = 0; j < dependingNodeIndexVec.size(); j ++)
		{
			size_t dependingNodeInd = nodesVec[i].DependingNodeIndex[j];
			for (int frameIndex = 0; frameIndex < gSwapChainImageCount; frameIndex++)
			{
				static_cast<VulkanPipelineNode*>(mRenderingNodesVec[i].get())->
					AddWaitSemaphore(static_cast<VulkanPipelineNode*>(mRenderingNodesVec[dependingNodeInd].get())->GetSignalSemaphore(frameIndex), frameIndex);
			}
		}
	}
}

void VulkanRenderingPipeline::TopologySort(std::vector<RenderingPipelineNodeDesc>& nodesVec, std::vector<std::vector<int>> dependingMap)
{
	std::vector<RenderingPipelineNodeDesc> resultVec;
	resultVec.resize(nodesVec.size());
	std::vector<bool> addedVec(nodesVec.size(), false);
	std::vector<int> inVec(nodesVec.size(), 0);
	std::vector<int> prevToEndMap(nodesVec.size());
	int resultIndex = 0;
	for (size_t i = 0; i < nodesVec.size(); i ++)
	{
		inVec[i] = nodesVec[i].DependingNodeIndex.size();
	}

	size_t loopCount = 0;

	while (true)
	{
		for (size_t i = 0; i < nodesVec.size(); i++)
		{
			if (inVec[i] == 0 && addedVec[i] == false)
			{
				resultVec[resultIndex] = nodesVec[i];
				prevToEndMap[i] = resultIndex;
				resultIndex++;
				addedVec[i] = true;
				for (size_t k = 0; k < nodesVec.size(); k++)
				{
					inVec[k] -= dependingMap[i][k];
				}
			}
		}

		if (resultIndex == nodesVec.size())
		{
			break;
		}
		loopCount++;
		if (nodesVec.size() == loopCount && resultVec.size() < nodesVec.size())
		{
			throw "Rendering Map Has Dead Lock.";
		}
	}

	for (size_t i = 0; i < resultVec.size(); i++)
	{
		for (size_t j = 0; j < resultVec[i].DependingNodeIndex.size(); j ++)
		{
			resultVec[i].DependingNodeIndex[j] = prevToEndMap[resultVec[i].DependingNodeIndex[j]];
		}
	}

	nodesVec.swap(resultVec);
}

void VulkanRenderingPipeline::SubmitRenderingCommands(int frameIndex, VkQueue graphicsQueue)
{
	std::vector<VkSubmitInfo> submitVec;
	submitVec.reserve(mRenderingNodesVec.size());
	for (auto node : mRenderingNodesVec)
	{
		auto commandBuffer = std::dynamic_pointer_cast<VulkanPipelineNode>(node)->RecordCommandBuffer(frameIndex);
		auto& waitSema = std::dynamic_pointer_cast<VulkanPipelineNode>(node)->GetWaitSemaphore(frameIndex);
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.waitSemaphoreCount = waitSema.size();
		if (submitInfo.waitSemaphoreCount > 0)
		{
			submitInfo.pWaitSemaphores = waitSema.data();
			std::vector<VkPipelineStageFlags> waitFlags;
			waitFlags.resize(waitSema.size(), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
			submitInfo.pWaitDstStageMask = waitFlags.data(); // to be modified
		}
		auto signalSema = std::dynamic_pointer_cast<VulkanPipelineNode>(node)->GetSignalSemaphore(frameIndex);
		if (signalSema != VK_NULL_HANDLE)
		{
			submitInfo.pSignalSemaphores = &signalSema;
			submitInfo.signalSemaphoreCount = 1;
		}
		submitVec.push_back(submitInfo);
	}
	vkQueueSubmit(graphicsQueue, submitVec.size(), submitVec.data(), VK_NULL_HANDLE);
}
