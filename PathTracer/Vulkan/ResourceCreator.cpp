#include "ResourceCreator.h"
#include "Image.h"

std::map<std::string, VulkanShader::VulkanShaderPtr> shaderMap;
std::map<std::string, IImage::ImagePtr> imageMap;
std::map<std::string, std::shared_ptr<VulkanMeshData>> meshDataMap;
std::map<std::string, VulkanAttachment::AttachmentPtr> attachmentMap;
std::map<std::string, IMesh::MeshPtr> meshMap;
std::map<std::string, VulkanMeshInstance::MeshInstancePtr> meshInstanceMap;
std::map<std::string, VulkanMaterial::MaterialPtr> materialMap;
std::map<std::string, WorldObject::ObjectPtr> objectMap;
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

	IBuffer::BufferPtr stagingBuffer = std::make_shared<VulkanBuffer>(desc, true);
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
		imageFile = "./../../Asset/Dst/black.data";
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

IImage::ImagePtr ResourceCreator::CreateInnerImage(ImageDesc imageDesc, std::string imageName, int frameIndex)
{
	std::string frameImageName = imageName + std::to_string(frameIndex);
	if (imageMap.find(frameImageName) != imageMap.end())
	{
		return imageMap[frameImageName];
	}
	IImage::ImagePtr imgPtr = std::make_shared<VulkanImage>(imageDesc);
	imageMap[frameImageName] = imgPtr;
	return imgPtr;
}

IBuffer::BufferPtr ResourceCreator::CreateVertexBuffer(char * bufferData, UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = bufferData;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_VERTEX_BUFFER;

	IBuffer::BufferPtr vertexBuffer = std::make_shared<VulkanBuffer>(desc, true);
	return vertexBuffer;
}

IBuffer::BufferPtr ResourceCreator::CreateIndexBuffer(char * bufferData, UInt32 bufferSize)
{
	BufferDesc desc = {};
	desc.BufferData = bufferData;
	desc.Size = bufferSize;
	desc.Usage = BufferUsageBits::BU_INDEX_BUFFER;

	IBuffer::BufferPtr indexBuffer = std::make_shared<VulkanBuffer>(desc, true);
	return indexBuffer;
}

VulkanMaterial::MaterialPtr ResourceCreator::CreateMaterial(std::string materialName, std::string computeShader)
{
	auto getShaderName = [](std::string file)->std::string
	{
		return file.substr(file.find_last_of('\\') + 1);
	};

	if (materialMap.find(materialName) != materialMap.end())
	{
		return materialMap[materialName];
	}
	VulkanMaterialShader materialShader = {};
	materialShader.ComputeShader = CreateShaderFromFile(computeShader);
	materialMap[materialName] = std::make_shared<VulkanMaterial>(materialShader, getShaderName(computeShader));
	return materialMap[materialName];
}

VulkanMaterial::MaterialPtr ResourceCreator::CreateMaterial(std::string materialName, MaterialMode mode, std::string vertexShader, std::string fragmentShader, std::string geometryShader)
{
	auto getShaderName = [](std::string file)->std::string
	{
		return file.substr(file.find_last_of('\\') + 1);
	};

	if (materialMap.find(materialName) != materialMap.end())
	{
		return materialMap[materialName];
	}
	VulkanMaterialShader materialShader = {};
	materialShader.VertexShader = CreateShaderFromFile(vertexShader);
	materialShader.FragmentShader = CreateShaderFromFile(fragmentShader);
	materialShader.GeometryShader = CreateShaderFromFile(geometryShader);
	materialMap[materialName] = std::make_shared<VulkanMaterial>(materialShader, getShaderName(vertexShader) + getShaderName(fragmentShader) + getShaderName(geometryShader), mode);
	return materialMap[materialName];
}

VulkanMeshInstance::MeshInstancePtr ResourceCreator::CreateMeshFromFile(std::string modelDataFile, std::string vertexShaderFile, int meshIndex, std::string modelName)
{
	if (meshInstanceMap.find(modelName) != meshInstanceMap.end())
	{
		return meshInstanceMap[modelName];
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
		else if (input.name == "inTangent")
			meshChannels.push_back(ImportMeshData::MeshDataChannel::Tangent);
		else if (input.name == "inBiTangent")
			meshChannels.push_back(ImportMeshData::MeshDataChannel::BiTangent);
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

	std::string meshFile =  modelDataFile + std::to_string(meshIndex);
	if (meshMap.find(meshFile) == meshMap.end())
	{
		meshMap[meshFile] = meshDataMap[modelDataFile]->ExportVulkanMesh(meshIndex);
	}

	meshInstanceMap[modelName] = std::make_shared<VulkanMeshInstance>(meshMap[meshFile], meshChannels);
	return meshInstanceMap[modelName];
}

VulkanMeshInstance::MeshInstancePtr ResourceCreator::CreateInnerMesh(std::string modelName, std::vector<char>& vertBuffer, std::vector<char>& indBuffer, int vertCount, int indCount)
{
	if (meshInstanceMap.find(modelName) != meshInstanceMap.end())
	{
		return meshInstanceMap[modelName];
	}
	meshMap[modelName] = std::make_shared<VulkanMesh>(vertBuffer, indBuffer, vertCount, indCount);
	std::vector<ImportMeshData::MeshDataChannel> meshChannels =
	{
		ImportMeshData::MeshDataChannel::Position,
		ImportMeshData::MeshDataChannel::Normal,
		ImportMeshData::MeshDataChannel::Tangent,
		ImportMeshData::MeshDataChannel::BiTangent,
		ImportMeshData::MeshDataChannel::UV0
	};
	meshInstanceMap[modelName] = std::make_shared<VulkanMeshInstance>(meshMap[modelName], meshChannels);
	return meshInstanceMap[modelName];
}

IMesh::MeshPtr ResourceCreator::GetExportedMesh(std::string meshName)
{
	return meshInstanceMap[meshName]->GetMeshPtr();
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

VulkanAttachment::AttachmentPtr ResourceCreator::CreateColorAttachment(std::string name, int width, int height, TextureFormat format)
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
	desc.Format = format;
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
	if (shaderFile == "") return nullptr;
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

		if (shaderData->ShaderType == ShaderTypeEnum::ImportGeometryShader)
		{
			shader->SetPrimitive(shaderData->PrimitiveInput);
		}

		if (shaderData->ShaderType == ShaderTypeEnum::ImportComputeShader)
		{
			shader->SetLocalSize(shaderData->ComputeLocalSize);
		}
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

WorldObject::ObjectPtr ResourceCreator::CreateWorldObject(std::string objectName, std::string materialName, std::string modelName)
{
	assert(materialMap.find(materialName) != materialMap.end());
	assert(meshInstanceMap.find(modelName) != meshInstanceMap.end());
	if (objectMap[objectName] != nullptr) return objectMap[objectName];
	auto obj = std::make_shared<WorldObject>(materialName, modelName, objectName);
	objectMap.insert(std::make_pair(objectName, obj));
	return obj;
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

