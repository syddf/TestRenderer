#pragma once

#include "VulkanCommon.h"
#include "Buffer.h"
#include "./../../AssetImporter/ImportSceneData.h"

struct DefaultMeshStructure
{
	Vec3 Position;
	Vec3 Normal;
	Vec3 Tangent;
	Vec3 BiTangent;
	Vec2 TexCoord;
};

class VulkanMeshData
{
public:
	VulkanMeshData(ImportSceneData& importSceneData) : mImportSceneData(importSceneData) {};
	using MeshChannel = ImportMeshData::MeshDataChannel;

public:
	IMesh::MeshPtr ExportVulkanMesh(int meshIndex);

private:
	template<MeshChannel channel>
	void CopyChannelData(char*& data, int vertIndex)
	{
		auto value = mImportSceneData.MeshData.GetChannelData<channel>(vertIndex);
		memcpy(data, &value, sizeof(value));
		data += sizeof(value);
	}

private:
	ImportSceneData& mImportSceneData;
};

class VulkanMesh : public IMesh
{
public:
	VulkanMesh(std::vector<char>& vertexData, std::vector<char>& indexData, int vertexCount, int indexCount);
	~VulkanMesh();

public:
	char* GetVertexBufferGPUHandleAddress();
	char* GetIndexBufferGPUHandleAddress();
	int GetVertexCount();
	int GetIndexCount();
	int GetIndexBufferDataSize();

private:
	void CreateVertexBuffer(std::vector<char>& vertexData, int vertexCount);
	void CreateIndexBuffer(std::vector<char>& indexData, int indexCount);

private:
	VulkanBuffer::BufferPtr mVertexBuffer;
	VulkanBuffer::BufferPtr mIndexBuffer;
	int mVertexCount;
	int mIndexCount;
	int mIndexDataSize;
};

class VulkanMeshInstance
{
public:
	using MeshChannel = ImportMeshData::MeshDataChannel;
	using MeshInstancePtr = std::shared_ptr<VulkanMeshInstance>;
	VulkanMeshInstance(IMesh::MeshPtr meshPtr, std::vector<MeshChannel>& channels);


	IMesh::MeshPtr GetMeshPtr() { return mMeshPtr; }
private:
	IMesh::MeshPtr mMeshPtr;
	std::vector<MeshChannel> mMeshChannels;
};