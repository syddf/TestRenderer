#pragma once
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "VulkanCommon.h"

class VulkanBuffer : public IBuffer
{
public:
	VulkanBuffer(BufferDesc desc);
	~VulkanBuffer();

public:
	void CreateBuffer(BufferDesc desc);
	char* GetGPUBufferHandleAddress();
	VkDeviceMemory GetGPUBufferMemory() { return mBufferMemory; }
	int GetBufferSize() { return mBufferSize; };

private:
	VkBuffer mBuffer;
	VkDeviceMemory mBufferMemory;
	int mBufferSize;
};