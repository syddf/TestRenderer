#include "WorldObject.h"


WorldObject::WorldObject(std::string materialName, std::string modelName, std::string objectName)
{
	mMaterialName = materialName;
	mModelName = modelName;
	mObjectName = objectName;
}

WorldObject::~WorldObject()
{
}

void WorldObject::SetPosition(float x, float y, float z)
{
	mTransform.Position = Vec3(x, y, z);
}

void WorldObject::SetRotation(float x, float y, float z)
{
	mTransform.Rotation = Vec3(x, y, z);
}

void WorldObject::SetScale(float x, float y, float z)
{
	mTransform.Scale = Vec3(x, y, z);
}
