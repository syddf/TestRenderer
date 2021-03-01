#pragma once
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "VulkanCommon.h"

class VulkanBuffer : public IBuffer
{
public:
	VulkanBuffer(BufferDesc desc, bool staging = false);
	~VulkanBuffer();

public:
	void CreateBuffer(BufferDesc desc, bool staging);
	char* GetGPUBufferHandleAddress();
	VkDeviceMemory GetGPUBufferMemory(int& offset) 
	{ 
		offset = mOffset;  
		if (mBufferMemory != VK_NULL_HANDLE)
			return mBufferMemory;
		return sBufferMemory[mFlags]; 
	}
	int GetBufferSize() { return mBufferSize; };

private:
	VkBuffer mBuffer;
	static std::map<VkMemoryPropertyFlags, VkDeviceMemory> sBufferMemory;
	static std::map<VkMemoryPropertyFlags, int> sBufferMemoryOffset;
	int mBufferSize;
	int mOffset;
	VkMemoryPropertyFlags mFlags;
	VkDeviceMemory mBufferMemory;
};