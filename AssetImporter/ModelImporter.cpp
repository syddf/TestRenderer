#include "ModelImporter.h"
#include "ImportSceneData.h"

ModelImporter::ModelImporter()
{
}

ModelImporter::~ModelImporter()
{
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
	ImportAsset* ImportAsset = new ImportSceneData();
	ImportSceneData* ImportScene = static_cast<ImportSceneData*>(ImportAsset);

	for (UInt32 i = 0; i < LoadedScene->mNumMeshes; i++)
	{
		aiMesh* mesh = LoadedScene->mMeshes[i];
		ImportScene->MeshData.ReserveVertex(mesh->mNumVertices);
		ImportScene->MeshData.MeshVertexCount.push_back(mesh->mNumVertices);
		for (UInt32 vertIndex = 0; vertIndex < mesh->mNumVertices; vertIndex++)
		{
			if (mesh->HasPositions())
			{
				aiVector3D pos = mesh->mVertices[vertIndex];
				ImportScene->MeshData.PositionVec.push_back(Vec3(pos.x, pos.y, pos.z));
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
	}

	ImportScene->Serialize(TarFileName);
	return true;
}
