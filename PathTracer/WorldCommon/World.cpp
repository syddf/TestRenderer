#include "World.h"

World::World(const ImportSceneData& sceneData)
	:mMainCamera(sceneData.CameraData)
{
	UInt32 lightCount = sceneData.LightData.LightVec.size();
	for (auto i = 0u; i < lightCount; i++)
	{
		mLights[i] = Light(sceneData.LightData.LightVec[i]);
	}
}

World::~World()
{
}
