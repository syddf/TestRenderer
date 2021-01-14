#pragma once

#include "./../../Source/Prefix.h"
#include "./../../AssetImporter/ImportSceneData.h"

struct Light
{
public:
	Light();
	Light(const ImportLight& light);
	~Light();

public:
	void ResetDefaultDirectionalLight();

public:
	Vec3 mAmbient;
	Vec3 mDiffuse;
	Vec3 mSpecular;
	Vec3 mPosition;
	Vec3 mDirection;
	LightType mType;
	float mAngleInnerCone;
	float mAngleOuterCone;
};