#pragma once

#include "./../Source/Prefix.h"
#include "ImportAsset.h"
#include "./../Source/FileHelper.h"
#include <glm/gtc/quaternion.hpp>

struct ImportMeshData
{
	std::vector<Vec3> PositionVec;
	std::vector<Vec3> NormalVec;
	std::vector<Vec2> TexCoord0Vec;
	std::vector<Vec2> TexCoord1Vec;
	std::vector<UInt32> IndicesVec;
	std::vector<UInt32> MeshVertexCount;
	std::vector<UInt32> MeshIndexCount;
	std::vector<int> MaterialIndex;
	std::vector<Vec3> MinPointVec;
	std::vector<Vec3> MaxPointVec;

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

enum TextureType
{
	None,
	Diffuse,
	Specular,
	Ambient,
	Emissive,
	Height,
	Normals,
	Shininess,
	Opacity,
	Displacement,
	Lightmap,
	Reflection,
	Unknow,
	TYPE_MAX
};

struct ImportMaterial
{
	Vec3 Ambient;
	Vec3 Diffuse;
	Vec3 Specular;
	Vec3 Emissive;
	Vec3 Transparent;
	float Opacity;
	float Shininess;
	float RefractionIndex;
	std::array<std::string, TextureType::TYPE_MAX> TexturePath;

	void Serialize(std::fstream& FileStream) const
	{
		SerializeHelper<Vec3>()(FileStream, Ambient);
		SerializeHelper<Vec3>()(FileStream, Diffuse);
		SerializeHelper<Vec3>()(FileStream, Specular);
		SerializeHelper<Vec3>()(FileStream, Emissive);
		SerializeHelper<Vec3>()(FileStream, Transparent);
		SerializeHelper<float>()(FileStream, Opacity);
		SerializeHelper<float>()(FileStream, Shininess);
		SerializeHelper<float>()(FileStream, RefractionIndex);
		for (int i = 0; i < TextureType::TYPE_MAX; i++)
			SerializeHelper<std::string>()(FileStream, TexturePath[i]);
	}

	void Deserialize(std::fstream& FileStream)
	{
		DeserializeHelper<Vec3>()(FileStream, Ambient);
		DeserializeHelper<Vec3>()(FileStream, Diffuse);
		DeserializeHelper<Vec3>()(FileStream, Specular);
		DeserializeHelper<Vec3>()(FileStream, Emissive);
		DeserializeHelper<Vec3>()(FileStream, Transparent);
		DeserializeHelper<float>()(FileStream, Opacity);
		DeserializeHelper<float>()(FileStream, Shininess);
		DeserializeHelper<float>()(FileStream, RefractionIndex);
		for (int i = 0; i < TextureType::TYPE_MAX; i++)
			DeserializeHelper<std::string>()(FileStream, TexturePath[i]);

	}
};

struct ImportMaterialData
{
	std::vector<ImportMaterial> MaterialVec;
};

struct ImportCameraData
{
	float AspectRatio;
	float PlaneNear;
	float PlaneFar;
	float FOV;
	Vec3 LookAt;
	Vec3 Pos;
	Vec3 Up;
	Matrix ViewTransform;
};

enum LightType
{
	DirectionalLight,
	PointLight,
	SpotLight
};

struct ImportLight
{
	Vec3 Ambient;
	Vec3 Diffuse;
	Vec3 Specular;
	Vec3 Position;
	Vec3 Direction;
	LightType Type;
	float AngleInnerCone;
	float AngleOuterCone;
};

struct ImportLightData
{
	std::vector<ImportLight> LightVec;
};

struct ImportNode
{
	Matrix Transform;
	Vec3 MinPoint;
	Vec3 MaxPoint;
	std::vector<int> MeshIndex;
	std::vector<int> ChildNodeIndex;

	void Serialize(std::fstream& FileStream) const
	{
		SerializeHelper<Matrix>()(FileStream, Transform);
		SerializeHelper<Vec3>()(FileStream, MinPoint);
		SerializeHelper<Vec3>()(FileStream, MaxPoint);
		SerializeHelper<std::vector<int>>()(FileStream, MeshIndex);
		SerializeHelper<std::vector<int>>()(FileStream, ChildNodeIndex);
	}

	void Deserialize(std::fstream& FileStream)
	{
		DeserializeHelper<Matrix>()(FileStream, Transform);
		DeserializeHelper<Vec3>()(FileStream, MinPoint);
		DeserializeHelper<Vec3>()(FileStream, MaxPoint);
		DeserializeHelper<std::vector<int>>()(FileStream, MeshIndex);
		DeserializeHelper<std::vector<int>>()(FileStream, ChildNodeIndex);
	}
};

struct ImportNodeData
{
	std::vector<ImportNode> NodeVec;
};

struct ImportSceneData : public ImportAsset
{
	ImportSceneData() : ImportAsset(AssetType::SceneData, GlobalAssetVersion)
	{

	};

	ImportMeshData MeshData;
	ImportMaterialData MaterialData;
	ImportCameraData CameraData;
	ImportLightData LightData;
	ImportNodeData NodeData;

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
		File.Write(MeshData.MaterialIndex);
		File.Write(MeshData.MinPointVec);
		File.Write(MeshData.MaxPointVec);
		File.Write(MaterialData.MaterialVec);
		File.Write(CameraData);
		File.Write(LightData.LightVec);
		File.Write(NodeData.NodeVec);
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
		File.Read(MeshData.MaterialIndex);
		File.Read(MeshData.MinPointVec);
		File.Read(MeshData.MaxPointVec);
		File.Read(MaterialData.MaterialVec);
		File.Read(CameraData);
		File.Read(LightData.LightVec);
		File.Read(NodeData.NodeVec);
	}
};