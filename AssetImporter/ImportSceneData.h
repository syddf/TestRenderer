#pragma once

#include "./../Source/Prefix.h"
#include "ImportAsset.h"
#include "./../Source/FileHelper.h"

struct ImportMeshData
{
	std::vector<Vec3> PositionVec;
	std::vector<Vec3> NormalVec;
	std::vector<Vec2> TexCoord0Vec;
	std::vector<Vec2> TexCoord1Vec;
	std::vector<UInt32> IndicesVec;
	std::vector<UInt32> MeshVertexCount;
	std::vector<UInt32> MeshIndexCount;

	template<int channel> auto GetChannelData(int index) const;

	enum MeshDataChannel
	{
		Position,
		Normal,
		UV0,
		UV1,
		UnknownVec3,
		UnknownVec2
	};

	void ReserveVertex(UInt32 Count)
	{
		UInt32 totalVerts = 0;
		for (auto vCount : MeshVertexCount)
			totalVerts += (UInt32)vCount;
		totalVerts += Count;
		PositionVec.reserve(totalVerts);
		NormalVec.reserve(totalVerts);
		TexCoord0Vec.reserve(totalVerts);
		TexCoord1Vec.reserve(totalVerts);
	}

	void ReserveIndex(UInt32 Count)
	{
		UInt32 totalIndices = 0;
		for (auto iCount : IndicesVec)
			totalIndices += (UInt32)iCount;
		totalIndices += Count;
		IndicesVec.reserve(totalIndices);
	}

	size_t GetMeshCount() const
	{
		return MeshVertexCount.size();
	}
};

template<>
inline auto ImportMeshData::GetChannelData<ImportMeshData::MeshDataChannel::Position>(int index) const 
{
	if (PositionVec.size() <= index) return Vec3(0, 0, 0);
	return PositionVec[index];
}

template<>
inline auto ImportMeshData::GetChannelData<ImportMeshData::MeshDataChannel::Normal>(int index) const 
{
	if (NormalVec.size() <= index) return Vec3(0, 0, 0);
	return NormalVec[index];
}

template<>
inline auto ImportMeshData::GetChannelData<ImportMeshData::MeshDataChannel::UV0>(int index) const
{
	if (TexCoord0Vec.size() <= index) return Vec2(0, 0);
	return TexCoord0Vec[index];
}

template<>
inline auto ImportMeshData::GetChannelData<ImportMeshData::MeshDataChannel::UV1>(int index) const
{
	if (TexCoord1Vec.size() <= index) return Vec2(0, 0);
	return TexCoord1Vec[index];
}

template<>
inline auto ImportMeshData::GetChannelData<ImportMeshData::MeshDataChannel::UnknownVec2>(int index) const
{
	return Vec2(0, 0);
}

template<>
inline auto ImportMeshData::GetChannelData<ImportMeshData::MeshDataChannel::UnknownVec3>(int index) const
{
	return Vec3(0, 0, 0);
}

struct ImportSceneData : public ImportAsset
{
	ImportSceneData() : ImportAsset(AssetType::SceneData, GlobalAssetVersion)
	{

	};

	ImportMeshData MeshData;

	void Serialize(const std::string& TarFileName)
	{
		FileHelper File(TarFileName, FileHelper::FileMode::FileWrite);
		File.Write(AssetTypeEnum);
		File.Write(AssetVersion);
		File.Write(MeshData.PositionVec);
		File.Write(MeshData.NormalVec);
		File.Write(MeshData.TexCoord0Vec);
		File.Write(MeshData.TexCoord1Vec);
		File.Write(MeshData.MeshVertexCount);
		File.Write(MeshData.IndicesVec);
		File.Write(MeshData.MeshIndexCount);
	}

	void Deserialize(const std::string& SrcFileName)
	{
		FileHelper File(SrcFileName, FileHelper::FileMode::FileRead);
		File.Read(AssetTypeEnum);
		File.Read(AssetVersion);
		File.Read(MeshData.PositionVec);
		File.Read(MeshData.NormalVec);
		File.Read(MeshData.TexCoord0Vec);
		File.Read(MeshData.TexCoord1Vec);
		File.Read(MeshData.MeshVertexCount);
		File.Read(MeshData.IndicesVec);
		File.Read(MeshData.MeshIndexCount);
	}
};