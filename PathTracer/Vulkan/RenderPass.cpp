#include "RenderPass.h"

extern VkDevice gVulkanDevice;

VulkanRenderingNode::VulkanRenderingNode(const RenderingPipelineNodeDesc & desc)
{
	mSignalSemaphore = VK_NULL_HANDLE;
	GenerateGraphicsNode(desc);
}

VulkanRenderingNode::~VulkanRenderingNode()
{
	vkDestroyFramebuffer(gVulkanDevice, mFrameBuffer, nullptr);
	vkDestroyRenderPass(gVulkanDevice, mVKRenderPass, nullptr);
	if (mSignalSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(gVulkanDevice, mSignalSemaphore, nullptr);
	}
}

void VulkanRenderingNode::GenerateGraphicsNode(const RenderingPipelineNodeDesc & desc)
{
	std::vector<VkAttachmentDescription> attachmentDesc;
	std::vector<VkAttachmentReference> colorAttachmentReference;
	VkAttachmentReference depthAttachmentReference;
	attachmentDesc.resize(desc.FrameBufferLayoutDesc.AttachmentDesc.size());

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
		}
		else if(desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage & TextureUsageBits::TU_DEPTH_STENCIL)
		{
			attachmentDesc[i].loadOp = GetVKLoadOp(AttachmentOperator::AO_DONT_CARE);
			attachmentDesc[i].storeOp = GetVKStoreOp(AttachmentOperator::AO_DONT_CARE);
			attachmentDesc[i].stencilLoadOp = GetVKLoadOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].LoadOp);
			attachmentDesc[i].stencilStoreOp = GetVKStoreOp(desc.FrameBufferLayoutDesc.AttachmentDesc[i].StoreOp);
		}
		attachmentDesc[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (desc.AttachToWindowNode)
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
		}
		else if (desc.FrameBufferLayoutDesc.AttachmentDesc[i].Usage == TextureUsageBits::TU_DEPTH_STENCIL)
		{
			depthAttachmentReference.attachment = i;
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = GetVKBindPoint(desc.BindPoint);
	subpassDesc.pColorAttachments = colorAttachmentReference.data();
	subpassDesc.colorAttachmentCount = colorAttachmentReference.size();
	subpassDesc.pDepthStencilAttachment = &depthAttachmentReference;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = desc.FrameBufferLayoutDesc.AttachmentDesc.size();
	renderPassCreateInfo.pAttachments = attachmentDesc.data();
	renderPassCreateInfo.pSubpasses = &subpassDesc;

	VKFUNC(vkCreateRenderPass(gVulkanDevice, &renderPassCreateInfo, nullptr, &mVKRenderPass), "Create Render Pass Failed.");

	std::vector<VkImageView> attachmentView;
	attachmentView.resize(desc.FrameBufferDesc.AttachmentImageViewHandlePtr.size());
	for (size_t i = 0; i < desc.FrameBufferDesc.AttachmentImageViewHandlePtr.size(); i ++)
	{
		attachmentView[i] = *reinterpret_cast<VkImageView*>(desc.FrameBufferDesc.AttachmentImageViewHandlePtr[i]);
	}
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.width = desc.FrameBufferDesc.Width;
	frameBufferInfo.height = desc.FrameBufferDesc.Height;
	frameBufferInfo.pAttachments = attachmentView.data();
	frameBufferInfo.attachmentCount = attachmentView.size();
	frameBufferInfo.layers = 1;
	VKFUNC(vkCreateFramebuffer(gVulkanDevice, &frameBufferInfo, nullptr, &mFrameBuffer), "Create Frame Buffer Failed.");
}

void VulkanRenderingNode::CreateSignalSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VKFUNC(vkCreateSemaphore(gVulkanDevice, &semaphoreCreateInfo, nullptr, &mSignalSemaphore), "Create Signal Semaphore Failed.");
}
