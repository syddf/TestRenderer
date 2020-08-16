#include "RenderingPipeline.h"
#include "RenderPass.h"

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
		for (size_t j = 0; j < nodesVec[i].FrameBufferDesc.AttachmentImageViewHandlePtr.size(); j ++)
		{
			VkImageView attachmentViewHandle = *reinterpret_cast<VkImageView*>(nodesVec[i].FrameBufferDesc.AttachmentImageViewHandlePtr[j]);
			mAttachmentAffectNodesMap[attachmentViewHandle].push_back(i);
		}
	}

	std::vector<std::vector<int>> graphEdgeMap;
	graphEdgeMap.resize(nodesVec.size(), std::vector<int>(nodesVec.size(), 0));
	for (size_t i = 0; i < nodesVec.size(); i ++)
	{
		for (size_t j = 0; j < nodesVec[i].DependingAttachmentViewPtr.size(); j ++)
		{
			VkImageView dependingImageViewHandle = *reinterpret_cast<VkImageView*>(nodesVec[i].DependingAttachmentViewPtr[j]);
			if (mAttachmentAffectNodesMap.find(dependingImageViewHandle) == mAttachmentAffectNodesMap.end())
			{
				throw "Unexpected Depending Attachment.";
			}
			else
			{
				auto dependingNodeVec = mAttachmentAffectNodesMap[dependingImageViewHandle];
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
		mRenderingNodesVec[i] = std::make_shared<VulkanRenderingNode>(nodesVec[i]);
	}

	for (size_t i = 0; i < nodesVec.size(); i++)
	{
		if (nodesVec[i].AffectOtherNode)
		{
			static_cast<VulkanRenderingNode*>(mRenderingNodesVec[i].get())->CreateSignalSemaphore();
		}
		auto dependingNodeIndexVec = nodesVec[i].DependingNodeIndex;
		for (size_t j = 0; j < dependingNodeIndexVec.size(); j ++)
		{
			size_t dependingNodeInd = nodesVec[i].DependingNodeIndex[j];
			static_cast<VulkanRenderingNode*>(mRenderingNodesVec[i].get())->
				AddWaitSemaphore(static_cast<VulkanRenderingNode*>(mRenderingNodesVec[dependingNodeInd].get())->GetSignalSemaphore());
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
