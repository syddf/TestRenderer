#pragma once

#include "./../../Source/Prefix.h"
#include "./../../AssetImporter/ImportSceneData.h"

class Camera
{
public:
	Camera(const ImportCameraData& cameraData);
	~Camera();

public:
	void GetViewTransformMatrix(Matrix& view, Matrix& proj);
	void SetParams(float aspect, float near, float far, float fov, Vec3 lookAt, Vec3 pos, Vec3 up);
	Vec3 GetPosition() const { return mPos; }

private:
	void UpdateViewTransform();
	void UpdateProjTransform();

private:
	float mAspectRatio;
	float mPlaneNear;
	float mPlaneFar;
	float mFOV;

	Vec3 mLookAt;
	Vec3 mPos;
	Vec3 mUp;
	Matrix mViewTransform;
	Matrix mProjTransform;
	bool mDirty;
};