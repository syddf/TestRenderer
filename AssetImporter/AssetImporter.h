#pragma once

#include "ModelImporter.h"
#include "ImportAsset.h"
#include "./../Source/PathHelper.h"

class AssetImporter
{
public:
	AssetType GetAssetType(const std::string SuffixName)
	{
		if (SuffixName == "fbx")
		{
			return AssetType::SceneData;
		}
		return AssetType::ErrorType;
	}

	bool ImportAsset(const std::string& SrcAssetFile, const std::string& TarAssetFile)
	{
		std::string AssetSuffix = PathHelper::GetPathSuffix(SrcAssetFile);
		AssetType AssetType = GetAssetType(AssetSuffix);
		if (AssetType == AssetType::ErrorType)
			return false;
		else if (AssetType == AssetType::SceneData)
			return ModelImporter.Load(SrcAssetFile, TarAssetFile);
	}

private:
	ModelImporter ModelImporter;
};