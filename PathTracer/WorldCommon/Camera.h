#pragma once

#include "./../../Source/Prefix.h"
#include "./../../AssetImporter/ImportSceneData.h"

class Camera
{
public:
	Camera(const ImportCameraData& cameraData);
	~Camera();

public:
	Matrix GetViewTransformMatrix();
	
private:
	void UpdateViewTransform();

private:
	const float mAspectRatio;
	const float mPlaneNear;
	const float mPlaneFar;
	const float mFOV;

	Vec3 mLookAt;
	Vec3 mPos;
	Vec3 mUp;
	Matrix mViewTransform;

	bool mDirty;
};