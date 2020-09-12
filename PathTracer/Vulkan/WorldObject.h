#pragma once
#include "VulkanCommon.h"

struct ObjectTransform
{
	Vec3 Position;
	Vec3 Rotation;
	Vec3 Scale;
};

struct WorldObject
{
public:
	WorldObject(std::string materialName, std::string modelName, std::string objectName);
	~WorldObject();

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

public:
	std::string mObjectName;
	std::string mMaterialName;
	std::string mModelName;
	ObjectTransform mTransform;
};