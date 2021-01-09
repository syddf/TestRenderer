#include "Camera.h"


Camera::Camera(const ImportCameraData & cameraData) : 
	mAspectRatio(cameraData.AspectRatio),
	mFOV(cameraData.FOV),
	mPlaneNear(cameraData.PlaneNear),
	mPlaneFar(cameraData.PlaneFar)
{
	mPos = cameraData.Pos;
	mUp = cameraData.Up;
	mLookAt = cameraData.LookAt;
	mViewTransform = cameraData.ViewTransform;
	mDirty = false;
}

Camera::~Camera()
{
}

Matrix Camera::GetViewTransformMatrix()
{
	if (mDirty)
		UpdateViewTransform();
	return mViewTransform;
}

void Camera::UpdateViewTransform()
{
	mViewTransform = glm::lookAtLH(mPos, mPos + mLookAt,  mUp);
	mDirty = true;
}
