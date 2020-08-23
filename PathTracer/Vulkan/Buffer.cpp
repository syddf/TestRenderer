#include "Buffer.h"
#include "ResourceCreator.h"
#include "TranslateEngine.h"

extern VkDevice gVulkanDevice;
extern ITranslationEngine* gTranslateEngine;

VulkanBuffer::VulkanBuffer(BufferDesc desc)
{
	CreateBuffer(desc);
}

VulkanBuffer::~VulkanBuffer()
{
	vkFreeMemory(gVulkanDevice, mBufferMemory, nullptr);
	vkDestroyBuffer(gVulkanDevice, mBuffer, nullptr);
}

void VulkanBuffer::CreateBuffer(BufferDesc desc)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = desc.Size;
	bufferCreateInfo.usage = GetVKBufferUsageFlags(desc.Usage);
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VKFUNC(vkCreateBuffer(gVulkanDevice, &bufferCreateInfo, nullptr, &mBuffer), "Failed To Create Buffer.");

	VkMemoryRequirements memoryRequire;
	vkGetBufferMemoryRequirements(gVulkanDevice, mBuffer, &memoryRequire);

	VkMemoryPropertyFlags propertyFlags = GetVKBufferMemoryProperty(desc.Usage);
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memoryRequire.size;
	memAllocInfo.memoryTypeIndex = GetVKMemoryType(memoryRequire.memoryTypeBits, propertyFlags);
	VKFUNC(vkAllocateMemory(gVulkanDevice, &memAllocInfo, nullptr, &mBufferMemory), "Failed To Allocate Memory.");
	vkBindBufferMemory(gVulkanDevice, mBuffer, mBufferMemory, 0);

	if (desc.BufferData != nullptr)
	{
		if (propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			void* data;
			VKFUNC(vkMapMemory(gVulkanDevice, mBufferMemory, 0, desc.Size, 0, &data), "Map Memory Failed.");
			memcpy(data, desc.BufferData, desc.Size);
			vkUnmapMemory(gVulkanDevice, mBufferMemory);
		}
		else if (propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			IBuffer::BufferPtr stagingBuffer = ResourceCreator::CreateStagingBuffer(desc.BufferData, desc.Size);
			VulkanTranslateEngine* transEngine = dynamic_cast<VulkanTranslateEngine*>(gTranslateEngine);
			TranslateBufferToBufferDesc transDesc;
			transDesc.GPUSrcBufferHandlePtr = stagingBuffer->GetGPUBufferHandleAddress();
			transDesc.GPUTarBufferHandlePtr = GetGPUBufferHandleAddress();
			transDesc.SrcOffset = 0;
			transDesc.DstOffset = 0;
			transDesc.CopySize = desc.Size;
			transEngine->TranslateBufferToBuffer(transDesc);
		}
	}

	mBufferSize = desc.Size;
}

char* VulkanBuffer::GetGPUBufferHandleAddress()
{
	return reinterpret_cast<char*>(&mBuffer);
}
