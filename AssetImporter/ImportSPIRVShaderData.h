#pragma once

#include "./../Source/Prefix.h"
#include "ImportAsset.h"
#include "./../Source/FileHelper.h"

enum ShaderTypeEnum
{
	ImportVertexShader,
	ImportFragmentShader,
	ImportComputeShader,
	ImportGeometryShader
};

enum GeometryPrimitiveInput
{
	PI_POINTS,
	PI_TRIANGLE
};

struct ShaderParameter
{
	int Count;
	int Offset;
	std::string Name;
	std::string Format;
	bool Combined;

	void Serialize(std::fstream& FileStream) const
	{
		SerializeHelper<int>()(FileStream, Count);
		SerializeHelper<int>()(FileStream, Offset);
		SerializeHelper<std::string>()(FileStream, Name);
		SerializeHelper<std::string>()(FileStream, Format);
		SerializeHelper<bool>()(FileStream, Combined);
	}

	void Deserialize(std::fstream& FileStream)
	{
		DeserializeHelper<int>()(FileStream, Count);
		DeserializeHelper<int>()(FileStream, Offset);
		DeserializeHelper<std::string>()(FileStream, Name);
		DeserializeHelper<std::string>()(FileStream, Format);
		DeserializeHelper<bool>()(FileStream, Combined);
	}
};

struct ShaderBlockInfo
{
	std::vector<ShaderParameter> ParamsVec;
	int Set;
	int Binding;
	int BlockSize;
	bool StructBuffer;

	void Serialize(std::fstream& FileStream) const
	{
		SerializeHelper<std::vector<ShaderParameter>>()(FileStream, ParamsVec);
		SerializeHelper<int>()(FileStream, Set);
		SerializeHelper<int>()(FileStream, Binding);
		SerializeHelper<int>()(FileStream, BlockSize);
		SerializeHelper<bool>()(FileStream, StructBuffer);
	}

	void Deserialize(std::fstream& FileStream)
	{
		DeserializeHelper<std::vector<ShaderParameter>>()(FileStream, ParamsVec);
		DeserializeHelper<int>()(FileStream, Set);
		DeserializeHelper<int>()(FileStream, Binding);
		DeserializeHelper<int>()(FileStream, BlockSize);
		DeserializeHelper<bool>()(FileStream, StructBuffer);
	}

	friend bool operator < (const ShaderBlockInfo& b1, const ShaderBlockInfo& b2)
	{
		return b1.Set < b2.Set || (b1.Set == b2.Set && b1.Binding < b2.Binding);
	}
};

struct ShaderInputInfo
{
	int location;
	std::string format;
	std::string name;

	void Serialize(std::fstream& FileStream) const
	{
		SerializeHelper<int>()(FileStream, location);
		SerializeHelper<std::string>()(FileStream, format);
		SerializeHelper<std::string>()(FileStream, name);
	}

	void Deserialize(std::fstream& FileStream)
	{
		DeserializeHelper<int>()(FileStream, location);
		DeserializeHelper<std::string>()(FileStream, format);
		DeserializeHelper<std::string>()(FileStream, name);
	}

	friend bool operator < (const ShaderInputInfo& i1, const ShaderInputInfo& i2) 
	{
		return i1.location < i2.location;
	}
};

struct ShaderPushConstantInfo
{
	int offset;
	std::string format;
	std::string name;

	void Serialize(std::fstream& FileStream) const
	{
		SerializeHelper<int>()(FileStream, offset);
		SerializeHelper<std::string>()(FileStream, format);
		SerializeHelper<std::string>()(FileStream, name);
	}

	void Deserialize(std::fstream& FileStream)
	{
		DeserializeHelper<int>()(FileStream, offset);
		DeserializeHelper<std::string>()(FileStream, format);
		DeserializeHelper<std::string>()(FileStream, name);
	}

	friend bool operator < (const ShaderPushConstantInfo& c1, const ShaderPushConstantInfo& c2)
	{
		return c1.offset < c2.offset;
	}
};

struct ImportSPIRVShaderData : public ImportAsset
{
	ImportSPIRVShaderData() : ImportAsset(AssetType::SPIRVShaderData, GlobalAssetVersion)
	{

	};

	ShaderTypeEnum ShaderType;
	GeometryPrimitiveInput PrimitiveInput;
	std::vector<char> ShaderData;
	std::vector<ShaderBlockInfo> ShaderParamsBlockInfo;
	std::vector<ShaderInputInfo> ShaderInputInfo;
	std::vector<ShaderPushConstantInfo> ShaderPushConstantInfo;
	Vec3 ComputeLocalSize;

	void Serialize(const std::string& TarFileName)
	{
		FileHelper File(TarFileName, FileHelper::FileMode::FileWrite);
		File.Write(AssetTypeEnum);
		File.Write(AssetVersion);
		File.Write(ShaderType);
		File.Write(PrimitiveInput);
		File.Write(ShaderData);
		File.Write(ShaderParamsBlockInfo);
		File.Write(ShaderInputInfo);
		File.Write(ShaderPushConstantInfo);
		File.Write(ComputeLocalSize);
	}

	void Deserialize(const std::string& SrcFileName)
	{
		FileHelper File(SrcFileName, FileHelper::FileMode::FileRead);
		File.Read(AssetTypeEnum);
		File.Read(AssetVersion);
		File.Read(ShaderType);
		File.Read(PrimitiveInput);
		File.Read(ShaderData);
		File.Read(ShaderParamsBlockInfo);
		File.Read(ShaderInputInfo);
		File.Read(ShaderPushConstantInfo);
		File.Read(ComputeLocalSize);
	}
};