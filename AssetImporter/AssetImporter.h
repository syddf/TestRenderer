#pragma once

#include "ModelImporter.h"
#include "TextureImporter.h"
#include "SPIRVImporter.h"
#include "ImportAsset.h"
#include "./../Source/PathHelper.h"

class AssetImporter
{
public:
	AssetType GetAssetType(const std::string SuffixName)
	{
		if (SuffixName == "fbx" || SuffixName == "obj")
		{
			return AssetType::SceneData;
		}
		else if (SuffixName == "png")
		{
			return AssetType::TextureData;
		}
		else if (SuffixName == "spv")
		{
			return AssetType::SPIRVShaderData;
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
		else if (AssetType == AssetType::TextureData)
			return TextureImporter.Load2DTexture(SrcAssetFile, TarAssetFile);
		else if (AssetType == AssetType::SPIRVShaderData)
			return SPIRVImporter.LoadSPIRVShader(SrcAssetFile, TarAssetFile);
	}

private:
	ModelImporter ModelImporter;
	TextureImporter TextureImporter;
	SPIRVImporter SPIRVImporter;
};