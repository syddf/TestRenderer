#include "Mesh.h"
#include "ResourceCreator.h"

VulkanMesh::VulkanMesh(std::vector<char>& vertexData, std::vector<char>& indexData, int vertexCount, int indexCount)
{
	CreateVertexBuffer(vertexData, vertexCount);
	CreateIndexBuffer(indexData, indexCount);
}

VulkanMesh::~VulkanMesh()
{
}

char * VulkanMesh::GetVertexBufferGPUHandleAddress()
{
	return mVertexBuffer->GetGPUBufferHandleAddress();
}

char * VulkanMesh::GetIndexBufferGPUHandleAddress()
{
	return mIndexBuffer->GetGPUBufferHandleAddress();
}

int VulkanMesh::GetVertexCount()
{
	return mVertexCount;
}

int VulkanMesh::GetIndexCount()
{
	return mIndexCount;
}

int VulkanMesh::GetIndexBufferDataSize()
{
	return mIndexDataSize;
}

void VulkanMesh::CreateVertexBuffer(std::vector<char>& vertexData, int vertexCount)
{
	mVertexBuffer = ResourceCreator::CreateVertexBuffer(vertexData.data(), vertexData.size());
	mVertexCount = vertexCount;
}

void VulkanMesh::CreateIndexBuffer(std::vector<char>& indexData, int indexCount)
{
	mIndexBuffer = ResourceCreator::CreateIndexBuffer(indexData.data(), indexData.size());
	mIndexCount = indexCount;
	mIndexDataSize = indexData.size() / mIndexCount;
}

IMesh::MeshPtr VulkanMeshData::ExportVulkanMesh(int meshIndex)
{
	static std::map<MeshChannel, int> channelSize = 
	{
		{ MeshChannel::Normal, 12},
		{ MeshChannel::Position, 12},
		{ MeshChannel::Tangent, 12},
	    { MeshChannel::BiTangent, 12},
		{ MeshChannel::UV0, 8},
		{ MeshChannel::UV1, 8},
		{ MeshChannel::UnknownVec2, 8},
		{ MeshChannel::UnknownVec3, 12}
	};
	int vertOffset = 0;
	int indOffset = 0;
	int vertCount = mImportSceneData.MeshData.MeshVertexCount[meshIndex];
	int indCount = mImportSceneData.MeshData.MeshIndexCount[meshIndex];
	for (int i = 0; i < meshIndex; i++)
	{
		vertOffset += mImportSceneData.MeshData.MeshVertexCount[i];
		indOffset += mImportSceneData.MeshData.MeshIndexCount[i];
	}
	
	int vertStructSize = 0;

	std::vector<MeshChannel> channels = 
	{
		MeshChannel::Position,
		MeshChannel::Normal,
		MeshChannel::Tangent,
		MeshChannel::BiTangent,
		MeshChannel::UV0
	};

	for (auto channel : channels)
		vertStructSize += channelSize[channel];

	int vertexBufferSize = vertStructSize * vertCount;
	std::vector<char> vertBuffer;
	vertBuffer.resize(vertexBufferSize);
	char* vertData = vertBuffer.data();

	for (int i = 0; i < vertCount; i++)
	{
		for (int j = 0; j < channels.size(); j++)
		{
			if (channels[j] == MeshChannel::Position)
				CopyChannelData<MeshChannel::Position>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::Normal)
				CopyChannelData<MeshChannel::Normal>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::UV0)
				CopyChannelData<MeshChannel::UV0>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::UV1)
				CopyChannelData<MeshChannel::UV1>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::UnknownVec2)
				CopyChannelData< MeshChannel::UnknownVec2>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::UnknownVec3)
				CopyChannelData< MeshChannel::UnknownVec3>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::Tangent)
				CopyChannelData< MeshChannel::Tangent>(vertData, i + vertOffset);
			else if (channels[j] == MeshChannel::BiTangent)
				CopyChannelData< MeshChannel::BiTangent>(vertData, i + vertOffset);
		}
	}

	std::vector<char> indexBuffer;
	if (vertCount < std::numeric_limits<UInt16>::max())
	{
		indexBuffer.resize(indCount * sizeof(UInt16));
		char* indData = indexBuffer.data();
		for (int i = 0; i < indCount; i++)
		{
			UInt16 value = mImportSceneData.MeshData.IndicesVec[i + indOffset];
			memcpy(indData, &value, sizeof(UInt16));
			indData += sizeof(UInt16);
		}
	}
	else
	{
		indexBuffer.resize(indCount * sizeof(UInt32));
		char* indData = indexBuffer.data();
		for (int i = 0; i < indCount; i++)
		{
			UInt32 value = mImportSceneData.MeshData.IndicesVec[i + indOffset];
			memcpy(indData, &value, sizeof(UInt32));
			indData += sizeof(UInt32);
		}
	}

	return std::make_shared<VulkanMesh>(vertBuffer, indexBuffer, vertCount, indCount);
}

VulkanMeshInstance::VulkanMeshInstance(IMesh::MeshPtr meshPtr, std::vector<MeshChannel>& channels)
{
	mMeshPtr = meshPtr;
	mMeshChannels = channels;
}
