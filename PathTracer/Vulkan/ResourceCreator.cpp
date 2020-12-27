#include "ResourceCreator.h"
#include "Image.h"
#include "WorldObject.h"
std::map<std::string, VulkanShader::VulkanShaderPtr> shaderMap;
std::map<std::string, IImage::ImagePtr> imageMap;
std::map<std::string, std::shared_ptr<VulkanMeshData>> meshDataMap;
std::map<std::string, VulkanAttachment::AttachmentPtr> attachmentMap;
std::map<std::string, IMesh::MeshPtr> meshMap;
std::map<std::string, VulkanMaterial::MaterialPtr> materialMap;
std::map<std::string, WorldObject> objectMap;
extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;

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
	if (imageFile == "")
	{
		imageFile = "./../../Asset/Dst/red.data";
	}
	if (imageMap.find(imageFile) != imageMap.end())
	{
		return imageMap[imageFile];
	}
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

	imageMap[imageFile] = imgPtr;
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

VulkanMaterial::MaterialPtr ResourceCreator::CreateMaterial(std::string materialName, std::string vertexShader, std::string fragmentShader)
{
	if (materialMap.find(materialName) != materialMap.end())
	{
		return materialMap[materialName];
	}
	VulkanMaterialShader materialShader = {};
	materialShader.VertexShader = CreateShaderFromFile(vertexShader);
	materialShader.FragmentShader = CreateShaderFromFile(fragmentShader);
	return materialMap[materialName] = std::make_shared<VulkanMaterial>(materialShader);
}

IMesh::MeshPtr ResourceCreator::CreateMeshFromFile(std::string modelDataFile, std::string vertexShaderFile, int meshIndex, std::string modelName)
{
	if (meshMap.find(modelName) != meshMap.end())
	{
		return meshMap[modelName];
	}
	auto meshData = GetAsset<ImportSceneData>(modelDataFile);
	auto vertexShader = CreateShaderFromFile(vertexShaderFile);
	auto shaderType = vertexShader->GetShaderType();
	assert(shaderType == VulkanShaderType::VertexShader);
	auto& shaderParams = vertexShader->GetShaderParams();
	std::vector<ImportMeshData::MeshDataChannel> meshChannels;
	for (auto input : shaderParams.InputVec)
	{
		if (input.name == "inPosition")
			meshChannels.push_back(ImportMeshData::MeshDataChannel::Position);
		else if (input.name == "inNormal")
			meshChannels.push_back(ImportMeshData::MeshDataChannel::Normal);
		else if (input.name == "inTexCoord")
			meshChannels.push_back(ImportMeshData::MeshDataChannel::UV0);
		else if (input.name == "inTexCoord2")
			meshChannels.push_back(ImportMeshData::MeshDataChannel::UV1);
		else
		{
			if (input.format == "float+2")
				meshChannels.push_back(ImportMeshData::MeshDataChannel::UnknownVec2);
			else if (input.format == "float+3")
				meshChannels.push_back(ImportMeshData::MeshDataChannel::UnknownVec3);
		}
	}
	if (meshDataMap.find(modelDataFile) == meshDataMap.end())
	{
		meshDataMap[modelDataFile] = std::make_shared<VulkanMeshData>(*meshData);
	}
	return meshMap[modelName] = meshDataMap[modelDataFile]->ExportVulkanMesh(meshChannels, meshIndex);
}

VulkanAttachment::AttachmentPtr ResourceCreator::CreateDepthStencilAttachment(std::string name, int width, int height)
{
	if (width == 0 && height == 0)
	{
		width = gScreenWidth;
		height = gScreenHeight;
	}
	ImageDesc desc = {};
	desc.Dimension = TextureDimension::Texture2D;
	desc.Depth = 1;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.Format = TextureFormat::TF_D24US8;
	desc.GenerateMipMap = true;
	desc.ArrayLayers = 1;
	desc.Usage = TextureUsageBits::TU_DEPTH_STENCIL;
	VulkanAttachment::AttachmentPtr newAttach = std::make_shared<VulkanAttachment>(desc, false);
	attachmentMap[name] = newAttach;
	return newAttach;
}

VulkanAttachment::AttachmentPtr ResourceCreator::CreateColorAttachment(std::string name, int width, int height)
{
	if (width == 0 && height == 0)
	{
		width = gScreenWidth;
		height = gScreenHeight;
	}
	ImageDesc desc = {};
	desc.Dimension = TextureDimension::Texture2D;
	desc.Depth = 1;
	desc.Width = gScreenWidth;
	desc.Height = gScreenHeight;
	desc.MipLevels = 1;
	desc.Format = TextureFormat::TF_B8G8R8A8SRGB;
	desc.GenerateMipMap = true;
	desc.ArrayLayers = 1;
	desc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT | TextureUsageBits::TU_SHADER_RESOURCE;
	VulkanAttachment::AttachmentPtr newAttach = std::make_shared<VulkanAttachment>(desc, true);
	attachmentMap[name] = newAttach;
	return newAttach;
}

VulkanAttachment::AttachmentPtr ResourceCreator::RenameAttachment(std::string originName, std::string anotherName)
{
	if (attachmentMap.find(originName) == attachmentMap.end())
	{
		throw "Unexpected attachment Name";
	}
	attachmentMap[anotherName] = attachmentMap[originName];
	return attachmentMap[anotherName];
}

VulkanAttachment::AttachmentPtr ResourceCreator::GetAttachment(std::string attachName)
{
	assert(attachmentMap.find(attachName) != attachmentMap.end());
	return attachmentMap[attachName];
}

void ResourceCreator::SaveAttachment(std::string attachmentName, VulkanAttachment::AttachmentPtr attachmentPtr)
{
	attachmentMap[attachmentName] = attachmentPtr;
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

void ResourceCreator::CreateWorldObject(std::string objectName, std::string materialName, std::string modelName)
{
	assert(materialMap.find(materialName) != materialMap.end());
	assert(meshMap.find(modelName) != meshMap.end());
	WorldObject obj = WorldObject(materialName, modelName, objectName);
	objectMap.insert(std::make_pair(objectName, obj));
}

void ResourceCreator::DestroyCachingResource()
{
	shaderMap.clear();
	imageMap.clear();
	meshDataMap.clear();
	meshMap.clear();
	attachmentMap.clear();
	materialMap.clear();
}

