#include "ResourceCreator.h"
#include "Image.h"

IBuffer::BufferPtr ResourceCreator::CreateStagingBuffer(char* bufferData, UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = bufferData;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_TRANSFER_SRC;

	IBuffer::BufferPtr stagingBuffer = std::make_shared<VulkanBuffer>(desc);
	return stagingBuffer;
}

IImage::ImagePtr ResourceCreator::CreateImageFromFile(std::string imageFile)
{
	ImportTextureData importTextureData;
	importTextureData.Deserialize(imageFile);

	UInt32 mipmapCount = static_cast<UInt32>(std::floor(std::log2(std::max(importTextureData.TextureWidth, importTextureData.TextureHeight) + 1)));

	ImageDesc desc = {};
	desc.Dimension = GetTextureDimension(importTextureData.TextureType);
	desc.Depth = importTextureData.TextureDepth;
	desc.Width = importTextureData.TextureWidth;
	desc.Height = importTextureData.TextureHeight;
	desc.MipLevels = mipmapCount;
	desc.Format = GetImageImportDataFormat(importTextureData.TextureChannel);
	desc.GenerateMipMap = true;
	desc.ImageData = importTextureData.TextureData.data();
	if (importTextureData.TextureType == TextureTypeEnum::ImportTexture2D)
	{
		desc.ArrayLayers = 1;
	}
	desc.Usage = TextureUsageBits::TU_SHADER_RESOURCE | TextureUsageBits::TU_TRANSFER_SRC | TextureUsageBits::TU_TRANSFER_DST;
	IImage::ImagePtr imgPtr = std::make_shared<VulkanImage>(desc);
	return imgPtr;
}

IBuffer::BufferPtr ResourceCreator::CreateVertexBuffer(char * bufferData, UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = bufferData;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_VERTEX_BUFFER;

	IBuffer::BufferPtr vertexBuffer = std::make_shared<VulkanBuffer>(desc);
	return vertexBuffer;
}

IBuffer::BufferPtr ResourceCreator::CreateIndexBuffer(char * bufferData, UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = bufferData;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_INDEX_BUFFER;

	IBuffer::BufferPtr indexBuffer = std::make_shared<VulkanBuffer>(desc);
	return indexBuffer;
}

TextureDimension ResourceCreator::GetTextureDimension(TextureTypeEnum texType)
{
	if (texType == TextureTypeEnum::ImportTexture2D)
		return TextureDimension::Texture2D;
	else if (texType == TextureTypeEnum::ImportTexture3D)
		return TextureDimension::Texture3D;
	else if (texType == TextureTypeEnum::ImportTextureCube)
		return TextureDimension::TextureCube;
	return TextureDimension::None;
}
