#pragma once

#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "VulkanCommon.h"
#include "Buffer.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "./../../AssetImporter/ImportTextureData.h"
#include "./../../AssetImporter/ImportSPIRVShaderData.h"
#include "VulkanAttachment.h"
#include "./../WorldCommon/WorldObject.h"

class ResourceCreator
{
public:
	static IBuffer::BufferPtr CreateStagingBuffer(char* bufferData, UInt32 bufferSize);
	static IBuffer::BufferPtr CreateUniformBuffer(UInt32 bufferSize);
	static IImage::ImagePtr CreateImageFromFile(std::string imageFile);
	static IImage::ImagePtr CreateInnerImage(ImageDesc imageDesc, std::string imageName, int frameIndex);
	static IBuffer::BufferPtr CreateVertexBuffer(char* bufferData, UInt32 bufferSize);
	static IBuffer::BufferPtr CreateIndexBuffer(char* bufferData, UInt32 bufferSize);
	static VulkanMaterial::MaterialPtr CreateMaterial(std::string materialName, std::string computeShader);
	static VulkanMaterial::MaterialPtr CreateMaterial(std::string materialName, MaterialMode mode, std::string vertexShader = "", std::string fragmentShader = "", std::string geometryShader = "");
	static IMesh::MeshPtr CreateMeshFromFile(std::string modelDataFile, std::string vertexShaderFile, int meshIndex, std::string modelName);
	static IMesh::MeshPtr GetExportedMesh(std::string meshName);
	static VulkanAttachment::AttachmentPtr CreateDepthStencilAttachment(std::string name, int width, int height);
	static VulkanAttachment::AttachmentPtr CreateColorAttachment(std::string name, int width, int height);
	static VulkanAttachment::AttachmentPtr RenameAttachment(std::string originName, std::string anotherName);
	static VulkanAttachment::AttachmentPtr GetAttachment(std::string attachName);
	static void SaveAttachment(std::string attachmentName, VulkanAttachment::AttachmentPtr attachmentPtr);
	static VulkanShader::VulkanShaderPtr CreateShaderFromFile(std::string shaderFile);
	static TextureDimension GetTextureDimension(TextureTypeEnum texType);
	static WorldObject::ObjectPtr CreateWorldObject(std::string objectName, std::string materialName, std::string modelName);
	static TextureFormat GetImageImportDataFormat(int channel)
	{
		if (channel == 1)
			return TextureFormat::TF_R8SRGB;
		if (channel == 3)
			return TextureFormat::TF_R8G8B8SRGB;
		else if (channel == 4)
			return TextureFormat::TF_R8G8B8A8SRGB;
		return TextureFormat::TF_UNDEFINED;
	}
	static TextureFormat GetInnerImageDataFormat(std::string format)
	{
		if (format == "R32ui")
			return TextureFormat::TF_R32UInt;
		else if (format == "R8")
			return TextureFormat::TF_R8Unorm;
		else if (format == "Rgba8")
			return TextureFormat::TF_R8G8B8A8Unorm;
		else if (format == "Rg16ui")
			return TextureFormat::TF_RG16UInt;
		return TextureFormat::TF_R8G8B8A8Unorm;
	}
	static void DestroyCachingResource();

	template<typename T>
	inline static T* GetAsset(std::string assetFile)
	{
		static std::map<std::string, ImportAsset*> sAssetMap;
		if (sAssetMap.find(assetFile) == sAssetMap.end())
		{
			sAssetMap[assetFile] = new T();
			sAssetMap[assetFile]->Deserialize(assetFile);
		}
		return dynamic_cast<T*>(sAssetMap[assetFile]);
	}
};

