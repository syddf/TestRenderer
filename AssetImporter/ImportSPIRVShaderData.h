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

enum ShaderFormat
{
	SF_Char,
	SF_Short,
	SF_Int,
	SF_Float,
	SF_Bool,
	SF_Matrix,
	SF_Image
};

struct ShaderParameter
{
	int Count;
	int Offset;
	std::string Name;
	ShaderFormat Format;
};

struct ShaderBlockInfo
{
	std::vector<ShaderParameter> ParamsVec;
	std::string Name;
	int Set;
	int Binding;
	int BlockSize;
};

struct ImportSPIRVShaderData : public ImportAsset
{
	ImportSPIRVShaderData(ShaderTypeEnum shaderType) : ImportAsset(AssetType::SPIRVShaderData, GlobalAssetVersion)
	{

	};

	ShaderTypeEnum ShaderType;
	std::vector<char> ShaderData;
	std::vector<ShaderBlockInfo> ShaderParamsBlockInfo;

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