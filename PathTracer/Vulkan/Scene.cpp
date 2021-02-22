#include "Scene.h"
#include <glm/gtx/matrix_decompose.hpp>
VulkanSceneData::VulkanSceneData(std::string modelFile)
{
	mSceneFile = modelFile;
	mWorldData = std::make_shared<World>(*ResourceCreator::GetAsset<ImportSceneData>(mSceneFile));
}

VulkanSceneData::~VulkanSceneData()
{
}

std::vector<RenderingNodeDesc> VulkanSceneData::ExportAllRenderingNodeByMaterial(std::string vertexShaderFile, std::string fragShaderFile, std::string geometryShaderFile, std::string scenePrefix, MaterialMode materialMode)
{
	std::vector<RenderingNodeDesc> renderingNodeVec;
	auto meshData = ResourceCreator::GetAsset<ImportSceneData>(mSceneFile);
	auto& materialVec = meshData->MaterialData.MaterialVec;
	int matIndex = 0;
	renderingNodeVec.resize(materialVec.size());
	for (auto& renderingNode : renderingNodeVec)
	{
		renderingNode.World = mWorldData.get();
	}
	for (auto material : materialVec)
	{
		std::string materialName = this->GetSceneMaterialName(matIndex, scenePrefix);
		auto mat = ResourceCreator::CreateMaterial(materialName, materialMode, vertexShaderFile, fragShaderFile, geometryShaderFile);
		mWorldData->AddMaterialParams(mat);
		if (material.TexturePath[TextureType::Diffuse] != "")
			if(mat->HasImageParam("tDiffuse"))
				mat->SetImage("tDiffuse", material.TexturePath[TextureType::Diffuse]);	
		if (material.TexturePath[TextureType::Specular] != "")
			if (mat->HasImageParam("tSpecular"))
				mat->SetImage("tSpecular", material.TexturePath[TextureType::Specular]);
		if (material.TexturePath[TextureType::Height] != "")
			if (mat->HasImageParam("tNormal"))
				mat->SetImage("tNormal", material.TexturePath[TextureType::Height]);
		else if (material.TexturePath[TextureType::Normals] != "")
			if (mat->HasImageParam("tNormal"))
				mat->SetImage("tNormal", material.TexturePath[TextureType::Normals]);
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
			glm::quat rotation;
			glm::vec3 scale;
			glm::vec3 translate;
			glm::vec3 skew;
			glm::vec4 perspec;
			glm::decompose(node.Transform, scale, rotation, translate, skew, perspec);
			glm::vec3 eular = glm::eulerAngles(rotation);
			for (auto& meshNodeIndex : node.MeshIndex)
			{
				int materialIndex = meshData->MeshData.MaterialIndex[meshNodeIndex];
				auto objPtr = ResourceCreator::CreateWorldObject(GetSceneObjectName(objIndex, scenePrefix), GetSceneMaterialName(materialIndex, scenePrefix), GetSceneMeshName(meshNodeIndex, scenePrefix));
				objPtr->SetPosition(translate.x, translate.y, translate.z);
				objPtr->SetRotation(rotation.x, rotation.y, rotation.z);
				objPtr->SetScale(scale.x, scale.y, scale.z);
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

void VulkanSceneData::AddUpdateMaterial(std::string materialName)
{
	auto material = ResourceCreator::CreateMaterial(materialName, MaterialMode::Normal);
	mUpdateSceneDataMaterialVec.push_back(material);
	mWorldData->AddMaterialParams(material);
}

void VulkanSceneData::UpdateSceneData(int frameIndex)
{
	mWorldData->Update(frameIndex);
	Matrix viewMat = mWorldData->GetViewMatrix();
	Matrix projMat = mWorldData->GetProjMatrix();
	Light testLight = mWorldData->GetLight(0);

	float voxelSize;
	Vec3 minPoint;
	Matrix viewProj;
	int dimension;
	mWorldData->GetVoxelizationParams(dimension, viewProj, voxelSize, minPoint);

	for(auto material : mUpdateSceneDataMaterialVec)
	{
		/*
		material->SetMatrix("view", viewMat);
		material->SetMatrix("proj", projMat);
		material->SetFloat3("lights[0].diffuse", testLight.mDiffuse);
		material->SetFloat3("lights[0].specular", testLight.mSpecular);
		material->SetFloat3("lights[0].position", testLight.mPosition);
		material->SetFloat3("lights[0].direction", testLight.mDirection);		
		material->SetFloat3("uCameraPosition", mWorldData->GetCameraPosition());
		material->SetFloat("lightCount", 1.0f);
		*/
		/*
		material->SetFloat("voxelSize", voxelSize);
		material->SetFloat3("worldMinPoint", minPoint);
		material->SetMatrix("view", viewMat);
		material->SetMatrix("proj", projMat);
		material->SetFloat("dimension", dimension);
		*/
	}
}

void VulkanSceneData::InitializeSceneData()
{
	auto meshData = ResourceCreator::GetAsset<ImportSceneData>(mSceneFile);
}
