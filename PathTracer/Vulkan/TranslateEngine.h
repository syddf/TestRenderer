#pragma once
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "VulkanCommon.h"
#include "CommandBufferPool.h"

class VulkanTranslateEngine : public ITranslationEngine
{
public:
	~VulkanTranslateEngine();
	VulkanTranslateEngine(UInt32 TranslateQueueIndex, VkQueue translateQueue);

public:
	void TranslateBufferToTexture(TranslateBufferToImageDesc desc);
	void TranslateBufferToBuffer(TranslateBufferToBufferDesc desc);

private:
	VulkanCommandBufferPool::CommandPoolPtr mCommandPool;
};