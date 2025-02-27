#include "RenderPass.h"
#include "Material.h"
#include "CommandBufferPool.h"
#include "Mesh.h"
#include "VulkanAttachment.h"

extern VkDevice gVulkanDevice;
extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;
extern VulkanCommandBufferPool* gPipelineGraphicsSecondaryCommandBufferPool;
extern VulkanCommandBufferPool* gPipelineGraphicsPrimaryCommandBufferPool;
extern UInt32 gSwapChainImageCount;
extern std::map<std::string, VulkanAttachment::AttachmentPtr> attachmentMap;

VulkanPipelineNode::VulkanPipelineNode(const RenderingPipelineNodeDesc & desc)
{
	mSignalSemaphore.resize(gSwapChainImageCount, VK_NULL_HANDLE);
	mWaitSemaphore.resize(gSwapChainImageCount);
	mAttachmentWidth = desc.FrameBufferDesc.Width == 0 ? gScreenWidth : desc.FrameBufferDesc.Width;
	mAttachmentHeight = desc.FrameBufferDesc.Height == 0 ? gScreenHeight : desc.FrameBufferDesc.Height;

	mCommandBuffer.reserve(gSwapChainImageCount);
	for (int i = 0; i < gSwapChainImageCount; i++)
		mCommandBuffer.push_back(gPipelineGraphicsPrimaryCommandBufferPool->AllocateCommandBuffer(true));

	if (desc.BindPoint == PipelineBindPoint::BP_GRAPHICS)
	{
		GenerateGraphicsNode(desc);
		for (auto renderingNodeDesc : desc.RenderingNodeDescVec)
		{
			AddRenderingNodes(renderingNodeDesc);
		}
		mIsComputeNode = false;
	}
	else if (desc.BindPoint == PipelineBindPoint::BP_COMPUTE)
	{
		for (auto computeNodeDesc : desc.ComputeNodeDescVec)
		{
			AddComputeNodes(computeNodeDesc);
		}
		mIsComputeNode = true;
	}

	mNeedPresent = desc.AttachToWindowNode;
}

VulkanPipelineNode::~VulkanPipelineNode()
{
	if (!mIsComputeNode)
	{
		for (auto frameBuffer : mFrameBuffer)
			vkDestroyFramebuffer(gVulkanDevice, frameBuffer, nullptr);
		vkDestroyRenderPass(gVulkanDevice, mVKRenderPass, nullptr);
	}
	for (auto signalSemaphore : mSignalSemaphore)
	{
		if (signalSemaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(gVulkanDevice, signalSemaphore, nullptr);
		}
	}
}

void VulkanPipelineNode::GenerateGraphicsNode(const RenderingPipelineNodeDesc & desc)
{
	std::vector<VkAttachmentDescription> attachmentDesc;
	std::vector<VkAttachmentReference> colorAttachmentReference;
	VkAttachmentReference depthAttachmentReference = {};
	bool hasDepth = false;

	attachmentDesc.resize(desc.FrameBufferLayoutDesc.AttachmentDesc.size());

	mColorAttachmentCount = 0;
	for (size_t i = 0; i < desc.FrameBufferLayoutDesc.AttachmentDesc.size(); i++)
	{
		attachmentDesc[i] = {};
		attachmentDesc[i].format = GetVKTextureFormat(desc.FrameBufferLayoutDesc.AttachmentDesc[i].Format);
		if (desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage & TextureUsageBits::TU_COLOR_ATTACHMENT)
		{
			attachmentDesc[i].loadOp = GetVKLoadOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].LoadOp);
			attachmentDesc[i].storeOp = GetVKStoreOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].StoreOp);
			attachmentDesc[i].stencilLoadOp = GetVKLoadOp(AttachmentOperator::AO_DONT_CARE);
			attachmentDesc[i].stencilStoreOp = GetVKStoreOp(AttachmentOperator::AO_DONT_CARE);
			VkClearValue clearValue = {};
			clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };
			mClearValue.push_back(clearValue);
		}
		else if(desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage & TextureUsageBits::TU_DEPTH_STENCIL)
		{
			attachmentDesc[i].loadOp = GetVKLoadOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].LoadOp);
			attachmentDesc[i].storeOp = GetVKStoreOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].StoreOp);
			attachmentDesc[i].stencilLoadOp = GetVKLoadOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].LoadOp);
			attachmentDesc[i].stencilStoreOp = GetVKStoreOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].StoreOp);
			VkClearValue clearValue = {};
			clearValue.depthStencil = { 1.0f, 0 };
			mClearValue.push_back(clearValue);
		}
		attachmentDesc[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//		if (desc.)
		{
			if (desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage & TextureUsageBits::TU_COLOR_ATTACHMENT)
			{
				attachmentDesc[i].finalLayout = desc.AttachToWindowNode ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			else if (desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage & TextureUsageBits::TU_DEPTH_STENCIL)
			{
				attachmentDesc[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
		}
		attachmentDesc[i].samples = VK_SAMPLE_COUNT_1_BIT;

		if (desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage == TextureUsageBits::TU_COLOR_ATTACHMENT)
		{
			VkAttachmentReference colorReference = {};
			colorReference.attachment = i;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachmentReference.push_back(colorReference);
			mColorAttachmentCount++;
		}
		else if (desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage == TextureUsageBits::TU_DEPTH_STENCIL)
		{
			hasDepth = true;
			depthAttachmentReference.attachment = i;
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = GetVKBindPoint(desc.BindPoint);
	subpassDesc.pColorAttachments = colorAttachmentReference.data();
	subpassDesc.colorAttachmentCount = colorAttachmentReference.size();
	subpassDesc.pDepthStencilAttachment = hasDepth ? &depthAttachmentReference : nullptr;

	VkSubpassDependency dependency = {};
	dependency.dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;
	dependency.srcSubpass = 0;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = desc.FrameBufferLayoutDesc.AttachmentDesc.size();
	renderPassCreateInfo.pAttachments = attachmentDesc.data();
	renderPassCreateInfo.pSubpasses = &subpassDesc;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	VKFUNC(vkCreateRenderPass(gVulkanDevice, &renderPassCreateInfo, nullptr, &mVKRenderPass), "Create Render Pass Failed.");
	mFrameBuffer.resize(gSwapChainImageCount);
	for (size_t i = 0; i < desc.FrameBufferDesc.AttachmentName.size(); i++)
	{
		auto attach = attachmentMap[desc.FrameBufferDesc.AttachmentName[i]];
		mAttachment.push_back(attach);
	}

	for (int frameIndex = 0; frameIndex < gSwapChainImageCount; frameIndex++)
	{
		std::vector<VkImageView> attachmentView;
		attachmentView.reserve(desc.FrameBufferDesc.AttachmentName.size());
		for (size_t i = 0; i < desc.FrameBufferDesc.AttachmentName.size(); i++)
		{
			auto attach = attachmentMap[desc.FrameBufferDesc.AttachmentName[i]];
			attachmentView.push_back(*reinterpret_cast<VkImageView*>(attach->GetImage(frameIndex)->GetGPUImageViewHandleAddress()));
		}
		VkFramebufferCreateInfo frameBufferInfo = {};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		if (desc.FrameBufferDesc.AttachmentName.empty())
		{
			frameBufferInfo.width = gScreenWidth;
			frameBufferInfo.height = gScreenHeight;
		}
		else
		{
			frameBufferInfo.width = desc.FrameBufferDesc.Width;
			frameBufferInfo.height = desc.FrameBufferDesc.Height;
		}
		frameBufferInfo.pAttachments = attachmentView.data();
		frameBufferInfo.attachmentCount = attachmentView.size();
		frameBufferInfo.layers = 1;
		frameBufferInfo.renderPass = mVKRenderPass;

		VKFUNC(vkCreateFramebuffer(gVulkanDevice, &frameBufferInfo, nullptr, &mFrameBuffer[frameIndex]), "Create Frame Buffer Failed.");
	}
}

void VulkanPipelineNode::CreateSignalSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	for (int i = 0; i < gSwapChainImageCount; i++)
		VKFUNC(vkCreateSemaphore(gVulkanDevice, &semaphoreCreateInfo, nullptr, &mSignalSemaphore[i]), "Create Signal Semaphore Failed.");
}

void VulkanPipelineNode::AddRenderingNodes(RenderingNodeDesc desc)
{
	VulkanMaterial* material = reinterpret_cast<VulkanMaterial*>(desc.MaterialAddr);
	auto matMode = material->GetMaterialMode();
	if (matMode == MaterialMode::Normal)
	{
		std::vector<VkPipelineColorBlendAttachmentState> blendAttach;
		VkPipelineColorBlendAttachmentState blendAttachState = {};
		blendAttachState.blendEnable = VK_FALSE;
		blendAttachState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		for (int i = 0; i < mColorAttachmentCount; i++)
			blendAttach.push_back(blendAttachState);

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateInfo.attachmentCount = blendAttach.size();
		colorBlendStateInfo.pAttachments = blendAttach.data();
		colorBlendStateInfo.blendConstants[0] = 0.0f;
		colorBlendStateInfo.blendConstants[1] = 0.0f;
		colorBlendStateInfo.blendConstants[2] = 0.0f;
		colorBlendStateInfo.blendConstants[3] = 0.0f;
		mRenderingNodes.push_back(std::make_shared<VulkanRenderingNode>(desc, mVKRenderPass, colorBlendStateInfo, mAttachmentWidth, mAttachmentHeight));
	}
	else if (matMode == MaterialMode::NoAttachment)
	{
		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mRenderingNodes.push_back(std::make_shared<VulkanRenderingNode>(desc, mVKRenderPass, colorBlendStateInfo, mAttachmentWidth, mAttachmentHeight));
	}
}

void VulkanPipelineNode::AddComputeNodes(ComputeNodeDesc desc)
{
	if (desc.ComputeNode == nullptr)
		mComputeNodes.push_back(std::make_shared<VulkanComputeNode>(desc));
	else
		mComputeNodes.push_back(desc.ComputeNode);
}

VkCommandBuffer& VulkanPipelineNode::RecordCommandBuffer(int frameIndex)
{
	auto& commandBuffer = mCommandBuffer[frameIndex];
	gPipelineGraphicsPrimaryCommandBufferPool->BeginCommandBuffer(commandBuffer);

	if (!mRenderingNodes.empty())
	{
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.framebuffer = mFrameBuffer[frameIndex];
		renderPassBeginInfo.renderPass = mVKRenderPass;
		renderPassBeginInfo.clearValueCount = mClearValue.size();
		renderPassBeginInfo.pClearValues = mClearValue.data();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { (uint32_t)mAttachmentWidth, (uint32_t)mAttachmentHeight };
		VkSubpassContents subpassContents = mRenderingNodes.empty() ? VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE : VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, subpassContents);
		std::vector<VkCommandBuffer> childCommand;
		childCommand.reserve(mRenderingNodes.size());
		for (int i = 0; i < mRenderingNodes.size(); i++)
			childCommand.push_back(mRenderingNodes[i]->RecordCommandBuffer(frameIndex, renderPassBeginInfo));
		if (childCommand.size())
		{
			vkCmdExecuteCommands(commandBuffer, childCommand.size(), childCommand.data());
		}
		vkCmdEndRenderPass(commandBuffer);

		for (auto attach : mAttachment)
		{
			attach->TranslateAttachmentImage(frameIndex, mNeedPresent ? VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	if (!mComputeNodes.empty())
	{
		for (auto& computeNode : mComputeNodes)
		{
			computeNode->RecordCommandBuffer(commandBuffer, frameIndex);
		}
	}

	gPipelineGraphicsPrimaryCommandBufferPool->EndCommandBuffer(commandBuffer);
	return mCommandBuffer[frameIndex];
}

void VulkanRenderingNode::CreatePipeline(RenderingNodeDesc desc, VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo blendState, int width, int height)
{
	VulkanMaterial* material = reinterpret_cast<VulkanMaterial*>(desc.MaterialAddr);
	auto& descSetVec = material->GetSetLayouts();
	auto& pushConstantRangeVec = material->GetPushConstantRange();
	auto& shaderStageVec = material->GetShaderStageInfo();
	auto inputAssemblyState = material->GetInputAssemblyStateCreateInfo();
	std::vector<VkVertexInputAttributeDescription> inputAttrVec;
	VkVertexInputBindingDescription bindingDesc;
	auto vertexInputState = material->GetVertexInputStateCreateInfo(inputAttrVec, bindingDesc);
	auto depthStencilState = material->GetDepthStencilStateCreateInfo();
	auto rasterState = material->GetRasterizationStateCreateInfo();

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = width;
	scissor.extent.height = height;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineLayoutCreateInfo plCreateInfo = {};
	plCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plCreateInfo.pSetLayouts = descSetVec.data();
	plCreateInfo.setLayoutCount = descSetVec.size();
	plCreateInfo.pPushConstantRanges = pushConstantRangeVec.data();
	plCreateInfo.pushConstantRangeCount = pushConstantRangeVec.size();
	plCreateInfo.pPushConstantRanges = pushConstantRangeVec.data();
	VKFUNC(vkCreatePipelineLayout(gVulkanDevice, &plCreateInfo, nullptr, &mPipelineLayout), "Create Pipeline Layout Failed.");

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = shaderStageVec.size();
	graphicsPipelineCreateInfo.pStages = shaderStageVec.data();
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.layout = mPipelineLayout;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterState;
	graphicsPipelineCreateInfo.pColorBlendState = &blendState;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
	graphicsPipelineCreateInfo.pViewportState = &viewportState;
	VKFUNC(vkCreateGraphicsPipelines(gVulkanDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &mPipeline), "Create Pipeline Failed.");
}

void VulkanRenderingNode::GenerateCommandBuffer()
{
	mCommandBuffer.reserve(gSwapChainImageCount);
	for(int i = 0; i < gSwapChainImageCount; i ++)
		mCommandBuffer.push_back(gPipelineGraphicsSecondaryCommandBufferPool->AllocateCommandBuffer(false));
	mDirty.resize(3, true);
}

VkCommandBuffer VulkanRenderingNode::RecordCommandBuffer(int frameIndex, VkRenderPassBeginInfo renderPassInfo)
{
	auto commandBuffer = mCommandBuffer[frameIndex];
	if (mDirty[frameIndex] == false)
		return commandBuffer;
	gPipelineGraphicsSecondaryCommandBufferPool->BeginSecondaryCommandBuffer(commandBuffer, renderPassInfo);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
	
	if (mCustomNodePtr == nullptr)
	{
		VulkanMaterial* mat = reinterpret_cast<VulkanMaterial*>(mMaterialAddr);
		mat->Update(frameIndex);
		int descCount;
		std::vector<VkDescriptorSet> descSetVec = mat->GetDescriptorSet(frameIndex, descCount);
		VkDescriptorSet worldSet = mWorld->GetWorldParamsSet(mat, frameIndex);
		if (mObject.empty())
		{
			VkDeviceSize offsets[] = { 0 };
			//vkCmdBindVertexBuffers(commandBuffer, 0, 1, VK_NULL_HANDLE, offsets);
			std::vector<VkDescriptorSet> submitDescSetVec = mat->GetDescriptorSet(frameIndex, descCount, true);
			if (worldSet != VK_NULL_HANDLE)
				submitDescSetVec.push_back(worldSet);
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, submitDescSetVec.size(), submitDescSetVec.data(), 0, nullptr);
			vkCmdDraw(commandBuffer, mEmptyVertexCount, 1, 0, 0);
		}
		else
		{
			for (auto objPtr : mObject)
			{
				std::vector<VkDescriptorSet> submitDescSetVec = descSetVec;
				objPtr->Update(frameIndex);
				VkDescriptorSet objSet = objPtr->GetDescSet(frameIndex);
				if (objSet != VK_NULL_HANDLE)
					submitDescSetVec.push_back(objSet);
				if (worldSet != VK_NULL_HANDLE)
					submitDescSetVec.push_back(worldSet);
				vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, submitDescSetVec.size(), submitDescSetVec.data(), 0, nullptr);
				IMesh::MeshPtr mesh = objPtr->GetMeshPtr();
				int vertCount = mesh->GetVertexCount();
				int indCount = mesh->GetIndexCount();
				int indDataSize = mesh->GetIndexBufferDataSize();
				VkBuffer* vertBuffer = reinterpret_cast<VkBuffer*>(mesh->GetVertexBufferGPUHandleAddress());
				VkBuffer* indBuffer = reinterpret_cast<VkBuffer*>(mesh->GetIndexBufferGPUHandleAddress());
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertBuffer, offsets);
				vkCmdBindIndexBuffer(commandBuffer, *indBuffer, 0, indDataSize == sizeof(UInt16) ? VkIndexType::VK_INDEX_TYPE_UINT16 : VkIndexType::VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, indCount, 1, 0, 0, 0);
			}
		}
	}
	else
	{
		mCustomNodePtr->RecordCommandBuffer(commandBuffer, frameIndex, mPipelineLayout);
	}

	gPipelineGraphicsSecondaryCommandBufferPool->EndCommandBuffer(commandBuffer);
	return commandBuffer;
}

VulkanRenderingNode::VulkanRenderingNode(RenderingNodeDesc desc, VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo blendState, int width, int height)
{
	CreatePipeline(desc, renderPass, blendState, width, height);
	mMaterialAddr = desc.MaterialAddr;
	mObject = desc.Object;
	mEmptyVertexCount = desc.EmptyVertexCount;
	GenerateCommandBuffer();
	mWorld = desc.World;
	mCustomNodePtr = desc.CustomNode;
}

VulkanRenderingNode::~VulkanRenderingNode()
{
	vkDestroyPipelineLayout(gVulkanDevice, mPipelineLayout, nullptr);
	vkDestroyPipeline(gVulkanDevice, mPipeline, nullptr);
}
