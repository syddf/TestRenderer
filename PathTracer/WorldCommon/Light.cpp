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
