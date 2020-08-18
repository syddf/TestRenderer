#pragma once

#include "./../Source/Prefix.h"
#include "ImportAsset.h"
#include "./../Source/FileHelper.h"

enum ShaderTypeEnum
{
	ImportVertexShader,
	ImportFragmentShader,
	ImportComputeShader
};

struct ImportSPIRVShaderData : public ImportAsset
{
	ImportSPIRVShaderData(ShaderTypeEnum shaderType) : ImportAsset(AssetType::SPIRVShaderData, GlobalAssetVersion)
	{

	};

	ShaderTypeEnum ShaderType;
	std::vector<char> ShaderData;

	void Serialize(const std::string& TarFileName)
	{
		FileHelper File(TarFileName, FileHelper::FileMode::FileWrite);
		File.Write(AssetTypeEnum);
		File.Write(AssetVersion);
		File.Write(ShaderType);
		File.Write(ShaderData);
	}

	void Deserialize(const std::string& SrcFileName)
	{
		FileHelper File(SrcFileName, FileHelper::FileMode::FileRead);
		File.Read(AssetTypeEnum);
		File.Read(AssetVersion);
		File.Read(ShaderType);
		File.Read(ShaderData);
	}
};