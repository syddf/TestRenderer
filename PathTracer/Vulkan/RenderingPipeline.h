#pragma once

#include "VulkanCommon.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"

class VulkanRenderingPipeline : public IRenderingPipelineGraph
{
public:
	VulkanRenderingPipeline();
	~VulkanRenderingPipeline();

public:
    void GenerateRenderingGraph(std::vector<RenderingPipelineNodeDesc>& nodesVec);
	
	void TopologySort(std::vector<RenderingPipelineNodeDesc>& nodesVec, std::vector<std::vector<int>> dependingMap);
	std::vector<VkSemaphore> SubmitRenderingCommands(int index, VkQueue sgraphicsQueue, VkFence renderFinishFence);

private:
	std::map<std::string, std::vector<int>> mAttachmentAffectNodesMap;
	std::vector<NodePtr> mRenderingNodesVec;
	std::vector<std::vector<VkSemaphore>> mRenderFinishSemaphore;
};