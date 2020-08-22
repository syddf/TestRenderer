#include "ResourceCreator.h"
#include "Image.h"

std::map<std::string, VulkanShader::VulkanShaderPtr> shaderMap;

VulkanShaderType GetVulkanShaderType(ShaderTypeEnum importShaderType)
{
	if (importShaderType == ShaderTypeEnum::ImportVertexShader)
		return VulkanShaderType::VertexShader;
	else if (importShaderType == ShaderTypeEnum::ImportFragmentShader)
		return VulkanShaderType::FragmentShader;
	return VulkanShaderType::ComputeShader;
}

IBuffer::BufferPtr ResourceCreator::CreateStagingBuffer(char* bufferData, UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = bufferData;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_TRANSFER_SRC;

	IBuffer::BufferPtr stagingBuffer = std::make_shared<VulkanBuffer>(desc);
	return stagingBuffer;
}

IBuffer::BufferPtr ResourceCreator::CreateUniformBuffer(UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = nullptr;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_UNIFORM_BUFFER;
	IBuffer::BufferPtr uniformBuffer = std::make_shared<VulkanBuffer>(desc);
	return uniformBuffer;
}

IImage::ImagePtr ResourceCreator::CreateImageFromFile(std::string imageFile)
{
	ImportTextureData* importTexturePtr = GetAsset<ImportTextureData>(imageFile);
	auto importTextureData = *importTexturePtr;

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

VulkanMaterial::MaterialPtr ResourceCreator::CreateMaterial(std::string vertexShader, std::string fragmentShader)
{
	VulkanMaterialShader materialShader = {};
	materialShader.VertexShader = CreateShaderFromFile(vertexShader);
	materialShader.FragmentShader = CreateShaderFromFile(fragmentShader);
	return std::make_shared<VulkanMaterial>(materialShader);
}

VulkanShader::VulkanShaderPtr ResourceCreator::CreateShaderFromFile(std::string shaderFile)
{
	if (shaderMap.find(shaderFile) == shaderMap.end())
	{
		ImportSPIRVShaderData* shaderData = new ImportSPIRVShaderData();
		shaderData->Deserialize(shaderFile);
		VulkanShaderParams params;
		params.BlockVec = shaderData->ShaderParamsBlockInfo;
		params.InputVec = shaderData->ShaderInputInfo;
		params.PushConstantVec = shaderData->ShaderPushConstantInfo;
		VulkanShader::VulkanShaderPtr shader = std::make_shared<VulkanShader>(shaderData->ShaderData, params, GetVulkanShaderType(shaderData->ShaderType));
		shaderMap[shaderFile] = shader;
	}
	return shaderMap[shaderFile];
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

void ResourceCreator::DestroyCachingResource()
{
	shaderMap.clear();
}

