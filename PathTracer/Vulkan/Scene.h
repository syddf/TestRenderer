#pragma once

#include "VulkanCommon.h"
#include "./../../AssetImporter/ImportSceneData.h"
#include "ResourceCreator.h"
#include "RenderPass.h"

class VulkanSceneData
{
public:
	VulkanSceneData(std::string sceneFile);
	~VulkanSceneData();

public:
	std::vector<RenderingNodeDesc> ExportAllRenderingNodeByMaterial(std::string vertexShaderFile, std::string fragShaderFile, std::string scenePrefix);
	std::string GetSceneMaterialName(int index, std::string prefix);
	std::string GetSceneMeshName(int index, std::string prefix);
	std::string GetSceneObjectName(int index, std::string prefix);
	void UpdateSceneData();

private:
	void InitializeSceneData();

private:
	std::string mSceneFile;
	std::vector<VulkanMaterial::MaterialPtr> mUpdateSceneDataMaterialVec;
};