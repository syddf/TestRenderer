#pragma once

#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "VulkanCommon.h"
#include "Buffer.h"
#include "./../../AssetImporter/ImportTextureData.h"

class ResourceCreator
{
public:
	static IBuffer::BufferPtr CreateStagingBuffer(char* bufferData, UInt32 bufferSize);
	static IImage::ImagePtr CreateImageFromFile(std::string imageFile);

	static TextureDimension GetTextureDimension(TextureTypeEnum texType);
	static TextureFormat GetImageImportDataFormat(int channel)
	{
		if (channel == 3)
			return TextureFormat::TF_R8G8B8SRGB;
		else if (channel == 4)
			return TextureFormat::TF_R8G8B8A8SRGB;
		return TextureFormat::TF_UNDEFINED;
	}
};