#pragma once

#include "./../Source/Prefix.h"

#define GlobalAssetVersion 0u

enum AssetType
{
	ErrorType,
	SceneData,
	TextureData,
	SPIRVShaderData
};

struct ImportAsset
{
public:
	using AssetVersionType = UInt32;

	ImportAsset(AssetType AssetType, AssetVersionType AssetVersion) :
		AssetTypeEnum(AssetType),
		AssetVersion(AssetVersion)
	{

	}

public:
	AssetType AssetTypeEnum;
	AssetVersionType AssetVersion;

	virtual void Serialize(const std::string& TarFileName) = 0;
	virtual void Deserialize(const std::string& SrcFileName) = 0;
};
