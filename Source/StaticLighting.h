#pragma once

#include "Prefix.h"
#include "AssetImporter/ImportSceneData.h"
#include "Geometry.h"

struct StaticLightingVertex
{
	Point Position;
	Vec3 Normal;
	Vec2 TexCoord0;
	Vec2 TexCoord1;

	friend StaticLightingVertex operator-(const StaticLightingVertex& V0, const StaticLightingVertex& V1)
	{
		StaticLightingVertex Result;
		Result.Position = V0.Position - V1.Position;
		Result.Normal = V0.Normal - V1.Normal;
		Result.TexCoord0 = V0.TexCoord0 - V1.TexCoord0;
		Result.TexCoord1 = V0.TexCoord1 - V1.TexCoord1;
		return Result;
	}

	friend StaticLightingVertex operator+(const StaticLightingVertex& V0, const StaticLightingVertex& V1)
	{
		StaticLightingVertex Result;
		Result.Position = V0.Position + V1.Position;
		Result.Normal = V0.Normal + V1.Normal;
		Result.TexCoord0 = V0.TexCoord0 + V1.TexCoord0;
		Result.TexCoord1 = V0.TexCoord1 + V1.TexCoord1;
		return Result;
	}

	friend StaticLightingVertex operator*(const StaticLightingVertex& V0, float scale)
	{
		StaticLightingVertex Result;
		Result.Position = V0.Position * scale;
		Result.Normal = V0.Normal * scale;
		Result.TexCoord0 = V0.TexCoord0 * scale;
		Result.TexCoord1 = V0.TexCoord1 * scale;
		return Result;
	}

	friend StaticLightingVertex operator/(const StaticLightingVertex& V0, float scale)
	{
		StaticLightingVertex Result;
		float inv = 1.0f / scale;
		Result.Position = V0.Position * inv;
		Result.Normal = V0.Normal * inv;
		Result.TexCoord0 = V0.TexCoord0 * inv;
		Result.TexCoord1 = V0.TexCoord1 * inv;
		return Result;
	}
};


struct StaticLightingMesh
{
public:
	using LightingMeshVertexVec = std::vector<StaticLightingVertex>;
	using LightingMeshIndicesVec = std::vector<UInt32>;

public:
	void InitFromImportMesh(const ImportSceneData& SceneData, int MeshIndex)
	{
		UInt32 VertexStartIndex = 0;
		UInt32 IndexStartIndex = 0;
		for (int i = 0; i < MeshIndex - 1; i++)
		{
			VertexStartIndex += SceneData.MeshData.MeshVertexCount[i];
			IndexStartIndex += SceneData.MeshData.MeshIndexCount[i];
		}
		UInt32 VertexCount = SceneData.MeshData.MeshVertexCount[MeshIndex];
		UInt32 IndexCount = SceneData.MeshData.MeshIndexCount[MeshIndex];
		Vertices.reserve(VertexCount);
		Indices.reserve(IndexCount);
		for (size_t vertIndex = VertexStartIndex; vertIndex < VertexStartIndex + VertexCount; vertIndex++)
		{
			StaticLightingVertex newVertex;
			newVertex.Position = SceneData.MeshData.GetChannelData<ImportMeshData::Position>(vertIndex);
			newVertex.Normal = SceneData.MeshData.GetChannelData<ImportMeshData::Normal>(vertIndex);
			newVertex.TexCoord0 = SceneData.MeshData.GetChannelData<ImportMeshData::UV0>(vertIndex);
			newVertex.TexCoord1 = SceneData.MeshData.GetChannelData<ImportMeshData::UV1>(vertIndex);
			Vertices.push_back(newVertex);
		}

		for (size_t indIndex = IndexStartIndex; indIndex < IndexStartIndex + IndexCount; indIndex++)
		{
			Indices.push_back(SceneData.MeshData.IndicesVec[indIndex]);
		}
	}

	void GetTriangleVec(std::vector<Triangle>& TriangleVec)
	{
		TriangleVec.resize(Indices.size() / 3);
		for (int i = 0; i < TriangleVec.size(); i ++)
		{
			TriangleVec[i].V0.Position = Vertices[Indices[i * 3]].Position;
			TriangleVec[i].V1.Position = Vertices[Indices[i * 3 + 1]].Position;
			TriangleVec[i].V2.Position = Vertices[Indices[i * 3 + 2]].Position;
			TriangleVec[i].V0.Normal = Vertices[Indices[i * 3]].Normal;
			TriangleVec[i].V1.Normal = Vertices[Indices[i * 3 + 1]].Normal;
			TriangleVec[i].V2.Normal = Vertices[Indices[i * 3 + 2]].Normal;

		}
	}

public:
	LightingMeshVertexVec Vertices;
	LightingMeshIndicesVec Indices;
};