#pragma once

#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "VulkanCommon.h"
#include "Buffer.h"
#include "Shader.h"
#include "Material.h"
#include "./../../AssetImporter/ImportTextureData.h"
#include "./../../AssetImporter/ImportSPIRVShaderData.h"

class ResourceCreator
{
public:
	static IBuffer::BufferPtr CreateStagingBuffer(char* bufferData, UInt32 bufferSize);
	static IBuffer::BufferPtr CreateUniformBuffer(UInt32 bufferSize);
	static IImage::ImagePtr CreateImageFromFile(std::string imageFile);
	static IBuffer::BufferPtr CreateVertexBuffer(char* bufferData, UInt32 bufferSize);
	static IBuffer::BufferPtr CreateIndexBuffer(char* bufferData, UInt32 bufferSize);
	static VulkanMaterial::MaterialPtr CreateMaterial(std::string vertexShader, std::string fragmentShader);

	static VulkanShader::VulkanShaderPtr CreateShaderFromFile(std::string shaderFile);
	static TextureDimension GetTextureDimension(TextureTypeEnum texType);
	static TextureFormat GetImageImportDataFormat(int channel)
	{
		if (channel == 3)
			return TextureFormat::TF_R8G8B8SRGB;
		else if (channel == 4)
			return TextureFormat::TF_R8G8B8A8SRGB;
		return TextureFormat::TF_UNDEFINED;
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

