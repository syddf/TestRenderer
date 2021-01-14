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

private:
	void SetCameraDefault(const ImportSceneData& sceneData);
	void AddDefaultDirectionalLight(int index);
	
public:
	Vec3 GetCameraPosition() const { return mMainCamera.GetPosition(); }
	Light GetLight(int index);
	Matrix GetViewMatrix();
	Matrix GetProjMatrix();
	void Update();
};