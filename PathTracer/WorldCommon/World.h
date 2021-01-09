#pragma once
#include "./../../Source/Prefix.h"
#include "Camera.h"
#include "Light.h"
#include "WorldObject.h"
#define MAX_LIGHT 256

class World
{
public:
	World(const ImportSceneData& sceneData);
	~World();

private:
	Camera mMainCamera;
	Light mLights[MAX_LIGHT];
	std::vector<WorldObject> mWorldObjectVec;
};