#include "Buffer.h"
#include "ResourceCreator.h"
#include "TranslateEngine.h"

extern VkDevice gVulkanDevice;
extern ITranslationEngine* gTranslateEngine;

std::map<VkMemoryPropertyFlags, VkDeviceMemory> VulkanBuffer::sBufferMemory = std::map<VkMemoryPropertyFlags, VkDeviceMemory>();
std::map<VkMemoryPropertyFlags, int> VulkanBuffer::sBufferMemoryOffset = std::map<VkMemoryPropertyFlags, int>();

VulkanBuffer::VulkanBuffer(BufferDesc desc, bool staging)
{
	mBufferMemory = VK_NULL_HANDLE;
	CreateBuffer(desc, staging);
}

VulkanBuffer::~VulkanBuffer()
{
	vkDestroyBuffer(gVulkanDevice, mBuffer, nullptr);
	if (mBufferMemory != VK_NULL_HANDLE)
		vkFreeMemory(gVulkanDevice, mBufferMemory, nullptr);
}

void VulkanBuffer::CreateBuffer(BufferDesc desc, bool staging)
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
	memAllocInfo.memoryTypeIndex = GetVKMemoryType(memoryRequire.memoryTypeBits, propertyFlags);
	memAllocInfo.allocationSize = memoryRequire.size;

	if (staging == false)
	{
		if (sBufferMemory.find(propertyFlags) == sBufferMemory.end())
		{
			memAllocInfo.allocationSize = 200 * 1024 * 1024;
			VkDeviceMemory memory;
			VKFUNC(vkAllocateMemory(gVulkanDevice, &memAllocInfo, nullptr, &memory), "Failed To Allocate Memory.");
			sBufferMemory[propertyFlags] = memory;
			sBufferMemoryOffset[propertyFlags] = 0;
		}

		if (sBufferMemoryOffset[propertyFlags] % memoryRequire.alignment != 0)
		{
			sBufferMemoryOffset[propertyFlags] = sBufferMemoryOffset[propertyFlags] / memoryRequire.alignment;
			sBufferMemoryOffset[propertyFlags]++;
			sBufferMemoryOffset[propertyFlags] *= memoryRequire.alignment;
		}
		vkBindBufferMemory(gVulkanDevice, mBuffer, sBufferMemory[propertyFlags], sBufferMemoryOffset[propertyFlags]);
	}
	else
	{
		VKFUNC(vkAllocateMemory(gVulkanDevice, &memAllocInfo, nullptr, &mBufferMemory), "Failed To Allocate Memory.");
		vkBindBufferMemory(gVulkanDevice, mBuffer, mBufferMemory, 0);
		mOffset = 0;
	}

	if (desc.BufferData != nullptr)
	{
		if (propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			void* data;
			VKFUNC(vkMapMemory(gVulkanDevice, staging ? mBufferMemory : sBufferMemory[propertyFlags], staging ? 0 : sBufferMemoryOffset[propertyFlags], desc.Size, 0, &data), "Map Memory Failed.");
			memcpy(data, desc.BufferData, desc.Size);
			vkUnmapMemory(gVulkanDevice, staging ? mBufferMemory : sBufferMemory[propertyFlags]);
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

	if (!staging)
	{
		mOffset = sBufferMemoryOffset[propertyFlags];
		sBufferMemoryOffset[propertyFlags] += memoryRequire.size;
		mFlags = propertyFlags;
	}

	mBufferSize = desc.Size;
}

char* VulkanBuffer::GetGPUBufferHandleAddress()
{
	return reinterpret_cast<char*>(&mBuffer);
}
