#include "ModelImporter.h"
#include "ImportSceneData.h"
#include <direct.h>
#include <glm/gtc/matrix_transform.hpp>
#include "TextureImporter.h"

std::string GetSceneName(std::string SrcFileName)
{
	int length = SrcFileName.length();
	std::string res;
	int i = length - 1;
	for (; i >= 0 && SrcFileName[i] != '.'; i--);
	i--;
	for (; i >= 0 && SrcFileName[i] != '/' && SrcFileName[i] != '\\'; i--) res.push_back(SrcFileName[i]);
	for (int j = 0, k = res.length() - 1; j < k; j++, k--) std::swap(res[j], res[k]);
	return res;
}

ModelImporter::ModelImporter()
{
}

ModelImporter::~ModelImporter()
{
}

void TraverseNode(std::vector<ImportNode>& nodeVec, int index, aiNode* sceneNode, Matrix parentTransform, const ImportMeshData& meshData)
{
	nodeVec.reserve(nodeVec.size() + sceneNode->mNumChildren);
	auto& node = nodeVec[index];

	Vec3 minP = Vec3(MAXF, MAXF, MAXF);
	Vec3 maxP = Vec3(-MAXF, -MAXF, -MAXF);
	for (UInt32 i = 0; i < sceneNode->mNumMeshes; i++)
	{
		node.MeshIndex.push_back(sceneNode->mMeshes[i]);
		minP = glm::min(minP, meshData.MinPointVec[i]);
		maxP = glm::max(maxP, meshData.MaxPointVec[i]);
	}

	int baseIndex = nodeVec.size();
	for (UInt32 i = 0; i < sceneNode->mNumChildren; i++)
	{
		node.ChildNodeIndex.push_back(baseIndex + i);
		nodeVec.push_back(ImportNode());
	}

	aiVector3D position, scale;
	aiQuaternion rotation;
	sceneNode->mTransformation.Decompose(scale, rotation, position);
	auto Position = Vec3(position.x, position.y, position.z);
	auto Rotation = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
	auto Scale = Vec3(scale.x, scale.y, scale.z);

	node.Transform = glm::translate(parentTransform, Position);
	node.Transform = node.Transform * glm::mat4_cast(Rotation);
	node.Transform = glm::scale(node.Transform, Scale);
	node.MinPoint = minP;
	node.MaxPoint = maxP;

	for (UInt32 i = 0; i < sceneNode->mNumChildren; i++)
	{
		TraverseNode(nodeVec, baseIndex + i, sceneNode->mChildren[i], node.Transform, meshData);
	}
}

bool ModelImporter::Load(const std::string & SrcFileName, const std::string & TarFileName)
{
	Assimp::Importer FbxImporter;
	const aiScene* LoadedScene = FbxImporter.ReadFile(SrcFileName, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!LoadedScene)
	{
		std::cout << "Load Scene Failed For : " + SrcFileName << std::endl;
		return false;
	}
	std::string SceneName = GetSceneName(SrcFileName);
	ImportAsset* ImportAsset = new ImportSceneData();
	ImportSceneData* ImportScene = static_cast<ImportSceneData*>(ImportAsset);

	for (UInt32 i = 0; i < LoadedScene->mNumMaterials; i++)
	{
		ImportScene->MaterialData.MaterialVec.clear();
		aiMaterial* material = LoadedScene->mMaterials[i];
		ImportMaterial importMaterial;
		aiColor3D vec;
		material->Get(AI_MATKEY_COLOR_AMBIENT, vec);
		importMaterial.Ambient = glm::vec3(vec.r, vec.g, vec.b);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, vec);
		importMaterial.Diffuse = glm::vec3(vec.r, vec.g, vec.b);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, vec);
		importMaterial.Emissive = glm::vec3(vec.r, vec.g, vec.b);
		material->Get(AI_MATKEY_COLOR_TRANSPARENT, vec);
		importMaterial.Transparent = glm::vec3(vec.r, vec.g, vec.b);
		material->Get(AI_MATKEY_COLOR_SPECULAR, vec);
		importMaterial.Specular = glm::vec3(vec.r, vec.g, vec.b);
		material->Get(AI_MATKEY_REFRACTI, importMaterial.RefractionIndex);
		material->Get(AI_MATKEY_SHININESS, importMaterial.Shininess);

		for (aiTextureType texType = aiTextureType_NONE; texType < aiTextureType_UNKNOWN; texType = aiTextureType(texType + 1))
		{
			int textureTypeCount = material->GetTextureCount(texType);
			if (textureTypeCount <= 0) continue;
			aiString texPath;
			if (material->GetTexture(static_cast<aiTextureType>(texType), 0, &texPath) == AI_SUCCESS)
			{
				std::string srcFile = SrcFileName + "\\..\\" + texPath.data;
				std::string dstFolder = TarFileName + "\\..\\Texture\\" + SceneName;
				mkdir(dstFolder.c_str());
				std::string dstFile = (dstFolder + std::string("\\") + texPath.data);
				dstFile = dstFile.substr(0, dstFile.find_last_of('.') + 1) + std::string("data");
				mkdir((dstFile + "\\..\\").c_str());
				TextureImporter texImporter;
				texImporter.Load2DTexture(srcFile, dstFile);
				importMaterial.TexturePath[texType] = dstFile;
			}
		}

		ImportScene->MaterialData.MaterialVec.push_back(importMaterial);
	}

	for (auto node : ImportScene->NodeData.NodeVec)
		if (node.MeshIndex.size() == 1)
			std::cout << node.MeshIndex[0] << std::endl;

	if (LoadedScene->mNumCameras > 0)
	{
		int ind = LoadedScene->mNumCameras - 1;
		ImportScene->CameraData.PlaneNear = LoadedScene->mCameras[ind]->mClipPlaneNear;
		ImportScene->CameraData.PlaneFar = LoadedScene->mCameras[ind]->mClipPlaneFar;
		ImportScene->CameraData.AspectRatio = LoadedScene->mCameras[ind]->mAspect;
		ImportScene->CameraData.FOV = LoadedScene->mCameras[ind]->mHorizontalFOV;
		ImportScene->CameraData.LookAt = Vec3(LoadedScene->mCameras[ind]->mLookAt.x, LoadedScene->mCameras[ind]->mLookAt.y, LoadedScene->mCameras[ind]->mLookAt.z);
		ImportScene->CameraData.Pos = Vec3(LoadedScene->mCameras[ind]->mPosition.x, LoadedScene->mCameras[ind]->mPosition.y, LoadedScene->mCameras[ind]->mPosition.z);
		ImportScene->CameraData.Up = Vec3(LoadedScene->mCameras[ind]->mUp.x, LoadedScene->mCameras[ind]->mUp.y, LoadedScene->mCameras[ind]->mUp.z);
		ImportScene->CameraData.ViewTransform = glm::lookAtLH(ImportScene->CameraData.Pos, ImportScene->CameraData.LookAt, ImportScene->CameraData.Up);
	}

	for (UInt32 i = 0; i < LoadedScene->mNumLights; i++)
	{
		ImportLight light;
		light.Ambient = Vec3(LoadedScene->mLights[i]->mColorAmbient.r, LoadedScene->mLights[i]->mColorAmbient.g, LoadedScene->mLights[i]->mColorAmbient.b);
		light.Diffuse = Vec3(LoadedScene->mLights[i]->mColorDiffuse.r, LoadedScene->mLights[i]->mColorDiffuse.g, LoadedScene->mLights[i]->mColorDiffuse.b);
		light.Specular = Vec3(LoadedScene->mLights[i]->mColorSpecular.r, LoadedScene->mLights[i]->mColorSpecular.g, LoadedScene->mLights[i]->mColorSpecular.b);
		light.Position = Vec3(LoadedScene->mLights[i]->mPosition.x, LoadedScene->mLights[i]->mPosition.y, LoadedScene->mLights[i]->mPosition.z);
		light.Direction = Vec3(LoadedScene->mLights[i]->mDirection.x, LoadedScene->mLights[i]->mDirection.y, LoadedScene->mLights[i]->mDirection.z);
		light.AngleInnerCone = LoadedScene->mLights[i]->mAngleInnerCone;
		light.AngleOuterCone = LoadedScene->mLights[i]->mAngleOuterCone;
		if (LoadedScene->mLights[i]->mType == aiLightSource_DIRECTIONAL)
			light.Type = LightType::DirectionalLight;
		else if (LoadedScene->mLights[i]->mType == aiLightSource_POINT)
			light.Type = LightType::PointLight;
		else if (LoadedScene->mLights[i]->mType == aiLightSource_SPOT)
			light.Type = LightType::SpotLight;
		else
			continue;
		ImportScene->LightData.LightVec.push_back(light);
	}

	for (UInt32 i = 0; i < LoadedScene->mNumMeshes; i++)
	{
		aiMesh* mesh = LoadedScene->mMeshes[i];
		ImportScene->MeshData.ReserveVertex(mesh->mNumVertices);
		ImportScene->MeshData.MeshVertexCount.push_back(mesh->mNumVertices);
		Vec3 minPoint = Vec3(MAXF, MAXF, MAXF);
		Vec3 maxPoint = Vec3(-MAXF, -MAXF, -MAXF);
		for (UInt32 vertIndex = 0; vertIndex < mesh->mNumVertices; vertIndex++)
		{
			if (mesh->HasPositions())
			{
				aiVector3D pos = mesh->mVertices[vertIndex];
				ImportScene->MeshData.PositionVec.push_back(Vec3(pos.x, pos.y, pos.z));
				minPoint = glm::min(minPoint, Vec3(pos.x, pos.y, pos.z));
				maxPoint = glm::max(maxPoint, Vec3(pos.x, pos.y, pos.z));
			}
			if (mesh->HasNormals())
			{
				aiVector3D norm = mesh->mNormals[vertIndex];
				ImportScene->MeshData.NormalVec.push_back(Vec3(norm.x, norm.y, norm.z));
			}
			if (mesh->HasTextureCoords(0))
			{
				aiVector3D texCoord0 = mesh->mTextureCoords[0][vertIndex];
				ImportScene->MeshData.TexCoord0Vec.push_back(Vec2(texCoord0.x, texCoord0.y));
			}
			if (mesh->HasTextureCoords(1))
			{
				aiVector3D texCoord1 = mesh->mTextureCoords[1][vertIndex];
				ImportScene->MeshData.TexCoord1Vec.push_back(Vec2(texCoord1.x, texCoord1.y));
			}
		}
		UInt32 indexCount = mesh->mNumFaces * 3;
		ImportScene->MeshData.ReserveIndex(indexCount);
		ImportScene->MeshData.MeshIndexCount.push_back(indexCount);
		for (UInt32 faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
		{
			ImportScene->MeshData.IndicesVec.push_back(mesh->mFaces[faceIndex].mIndices[0]);
			ImportScene->MeshData.IndicesVec.push_back(mesh->mFaces[faceIndex].mIndices[1]);
			ImportScene->MeshData.IndicesVec.push_back(mesh->mFaces[faceIndex].mIndices[2]);
		}
		ImportScene->MeshData.MaterialIndex.push_back(LoadedScene->mMeshes[i]->mMaterialIndex);
		ImportScene->MeshData.MinPointVec.push_back(minPoint);
		ImportScene->MeshData.MaxPointVec.push_back(maxPoint);
	}

	if (LoadedScene->mRootNode != nullptr)
	{
		ImportNode rootNode;
		ImportScene->NodeData.NodeVec.push_back(rootNode);
		Matrix Identity = glm::identity<Matrix>();
		TraverseNode(ImportScene->NodeData.NodeVec, 0, LoadedScene->mRootNode, Identity, ImportScene->MeshData);
	}

	ImportScene->Serialize(TarFileName);
	return true;
}
