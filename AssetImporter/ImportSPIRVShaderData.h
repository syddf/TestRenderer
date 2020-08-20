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

struct ShaderParameter
{
	int Count;
	int Offset;
	std::string Name;
	std::string Format;
};

struct ShaderBlockInfo
{
	std::vector<ShaderParameter> ParamsVec;
	int Set;
	int Binding;
	int BlockSize;
	bool StructBuffer;
};

struct ShaderInputInfo
{
	std::string format;
	std::string name;
	int location;
};

struct ShaderPushConstantInfo
{
	int offset;
	std::string format;
	std::string name;
};

struct ImportSPIRVShaderData : public ImportAsset
{
	ImportSPIRVShaderData(ShaderTypeEnum shaderType) : ImportAsset(AssetType::SPIRVShaderData, GlobalAssetVersion)
	{

	};

	ShaderTypeEnum ShaderType;
	std::vector<char> ShaderData;
	std::vector<ShaderBlockInfo> ShaderParamsBlockInfo;
	std::vector<ShaderInputInfo> ShaderInputInfo;
	std::vector<ShaderPushConstantInfo> ShaderPushConstantInfo;

	void Serialize(const std::string& TarFileName)
	{
		FileHelper File(TarFileName, FileHelper::FileMode::FileWrite);
		File.Write(AssetTypeEnum);
		File.Write(AssetVersion);
		File.Write(ShaderType);
		File.Write(ShaderData);
		File.Write(ShaderParamsBlockInfo);
		File.Write(ShaderInputInfo);
		File.Write(ShaderPushConstantInfo);
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