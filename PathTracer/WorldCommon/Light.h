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

	void GetLightCameraViewProj(Vec3 minPoint, Vec3 maxPoint, Matrix& view, Matrix& proj);

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