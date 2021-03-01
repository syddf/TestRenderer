#include "Light.h"

Light::Light()
{
}

Light::Light(const ImportLight & light)
{
	mType = light.Type;
	mAmbient = light.Ambient;
	mAngleInnerCone = light.AngleInnerCone;
	mAngleOuterCone = light.AngleOuterCone;
	mDiffuse = light.Diffuse;
	mSpecular = light.Specular;
	mPosition = light.Position;
	mDirection = light.Direction;
}

Light::~Light()
{
}

void Light::ResetDefaultDirectionalLight()
{
}

void Light::GetLightCameraViewProj(Vec3 minPoint, Vec3 maxPoint, Matrix& view, Matrix& proj)
{
	Vec3 center = (minPoint + maxPoint) * 0.5f;
	float length = glm::length(maxPoint - minPoint);
	float radius = length * 0.5f;

	Vec3 pos = center + mDirection * radius;
	proj = glm::ortho(-radius, radius, -radius, radius, -radius, 2.0f * radius);
	view = glm::lookAt(pos, center, Vec3(0, 0, 1));
}
