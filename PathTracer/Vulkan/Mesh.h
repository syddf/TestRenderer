#pragma once

#include "VulkanCommon.h"
#include "Buffer.h"
#include "./../../AssetImporter/ImportSceneData.h"

class VulkanMeshData
{
public:
	VulkanMeshData(ImportSceneData& importSceneData) : mImportSceneData(importSceneData) {};
	using MeshChannel = ImportMeshData::MeshDataChannel;

public:
	IMesh::MeshPtr ExportVulkanMesh(std::vector<MeshChannel>& channels, int meshIndex);

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