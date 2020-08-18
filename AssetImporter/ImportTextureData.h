#pragma once

#include "./../Source/Prefix.h"
#include "ImportAsset.h"
#include "./../Source/FileHelper.h"

enum TextureTypeEnum
{
	ImportTexture2D,
	ImportTexture3D,
	ImportTextureCube
};

struct ImportTextureData : public ImportAsset
{
	ImportTextureData() : ImportAsset(AssetType::TextureData, GlobalAssetVersion)
	{

	};

	TextureTypeEnum TextureType;
	UInt32 TextureWidth;
	UInt32 TextureHeight;
	UInt32 TextureDepth;
	UInt32 TextureChannel;
	std::vector<char> TextureData;

	void Serialize(const std::string& TarFileName)
	{
		FileHelper File(TarFileName, FileHelper::FileMode::FileWrite);
		File.Write(AssetTypeEnum);
		File.Write(AssetVersion);
		File.Write(TextureType);
		File.Write(TextureWidth);
		File.Write(TextureHeight);
		File.Write(TextureDepth);
		File.Write(TextureChannel);
		File.Write(TextureData);
	}

	void Deserialize(const std::string& SrcFileName)
	{
		FileHelper File(SrcFileName, FileHelper::FileMode::FileRead);
		File.Read(AssetTypeEnum);
		File.Read(AssetVersion);
		File.Read(TextureType);
		File.Read(TextureWidth);
		File.Read(TextureHeight);
		File.Read(TextureDepth);
		File.Read(TextureChannel);
		File.Read(TextureData);
	}
};