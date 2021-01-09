#include "Scene.h"

VulkanSceneData::VulkanSceneData(std::string modelFile)
{
	mSceneFile = modelFile;
}

VulkanSceneData::~VulkanSceneData()
{
}

std::vector<RenderingNodeDesc> VulkanSceneData::ExportAllRenderingNodeByMaterial(std::string vertexShaderFile, std::string fragShaderFile, std::string scenePrefix)
{
	std::vector<RenderingNodeDesc> renderingNodeVec;
	auto meshData = ResourceCreator::GetAsset<ImportSceneData>(mSceneFile);
	auto& materialVec = meshData->MaterialData.MaterialVec;
	int matIndex = 0;
	renderingNodeVec.resize(materialVec.size());
	for (auto material : materialVec)
	{
		std::string materialName = this->GetSceneMaterialName(matIndex, scenePrefix);
		auto mat = ResourceCreator::CreateMaterial(materialName, vertexShaderFile, fragShaderFile);
		if (material.TexturePath[TextureType::Diffuse] != "")
			mat->SetImage("tDiffuse", material.TexturePath[TextureType::Diffuse]);	
		if (material.TexturePath[TextureType::Specular] != "")
			mat->SetImage("tSpecular", material.TexturePath[TextureType::Specular]);
		renderingNodeVec[matIndex].MaterialAddr = (char*)(mat.get());
		matIndex++;
		mUpdateSceneDataMaterialVec.push_back(mat);
	}
	auto meshCount = meshData->MeshData.MaterialIndex.size();
	for (int i = 0; i < meshCount; i ++)
	{
		std::string modelName = this->GetSceneMeshName(i, scenePrefix);
		ResourceCreator::CreateMeshFromFile(mSceneFile, vertexShaderFile, i, modelName);
	}

	int objIndex = 0;
	for (auto& node : meshData->NodeData.NodeVec)
	{
		if (node.ChildNodeIndex.size() == 0)
		{
			for (auto& meshNodeIndex : node.MeshIndex)
			{
				int materialIndex = meshData->MeshData.MaterialIndex[meshNodeIndex];
				auto objPtr = ResourceCreator::CreateWorldObject(GetSceneObjectName(objIndex, scenePrefix), GetSceneMaterialName(materialIndex, scenePrefix), GetSceneMeshName(meshNodeIndex, scenePrefix));
				renderingNodeVec[materialIndex].Object.push_back(objPtr);
				objIndex++;
			}
		}
	}
	return renderingNodeVec;
}

std::string VulkanSceneData::GetSceneMaterialName(int index, std::string prefix)
{
	return prefix + std::string("_material_") + std::to_string(index);
}

std::string VulkanSceneData::GetSceneMeshName(int index, std::string prefix)
{
	return prefix + std::string("_mesh_") + std::to_string(index);
}

std::string VulkanSceneData::GetSceneObjectName(int index, std::string prefix)
{
	return prefix + std::string("_object_") + std::to_string(index);
}

void VulkanSceneData::UpdateSceneData()
{

}

void VulkanSceneData::InitializeSceneData()
{
	auto meshData = ResourceCreator::GetAsset<ImportSceneData>(mSceneFile);
}
