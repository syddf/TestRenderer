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

void Camera::GetViewTransformMatrix(Matrix& view, Matrix& proj)
{
	if (mDirty)
	{
		UpdateViewTransform();
		UpdateProjTransform();
		mDirty = true;
	}
	view = mViewTransform;
	proj = mProjTransform;
}


void Camera::SetParams(float aspect, float near, float far, float fov, Vec3 lookAt, Vec3 pos, Vec3 up)
{
	mAspectRatio = aspect;
	mPlaneNear = near;
	mPlaneFar = far;
	mFOV = fov;
	mPos = pos;
	mLookAt = lookAt;
	mUp = up;
	mDirty = true;
}

void Camera::UpdateViewTransform()
{
	mViewTransform = glm::lookAt(mPos, mPos + mLookAt,  mUp);
}

void Camera::UpdateProjTransform()
{
	mProjTransform = glm::perspective(mFOV, mAspectRatio, mPlaneNear, mPlaneFar);
}
