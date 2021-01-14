#include "World.h"
extern UInt32 gScreenWidth;
extern UInt32 gScreenHeight;

World::World(const ImportSceneData& sceneData)
	:mMainCamera(sceneData.CameraData)
{
	UInt32 lightCount = sceneData.LightData.LightVec.size();
	for (auto i = 0u; i < lightCount; i++)
	{
		mLights[i] = Light(sceneData.LightData.LightVec[i]);
	}

	if (lightCount == 0)
	{
		SetCameraDefault(sceneData);
		AddDefaultDirectionalLight(0);
	}
}

World::~World()
{
}

void World::SetCameraDefault(const ImportSceneData& sceneData)
{
	auto meshCount = sceneData.MeshData.MinPointVec.size();
	Vec3 minPoint = Vec3(MAXF, MAXF, MAXF);
	Vec3 maxPoint = Vec3(-MAXF, -MAXF, -MAXF);
	for (auto i = 0; i < meshCount; i++)
	{
		minPoint = glm::min(sceneData.MeshData.MinPointVec[i], minPoint);
		maxPoint = glm::max(sceneData.MeshData.MaxPointVec[i], maxPoint);
	}
	Vec3 centerPoint = (minPoint + maxPoint) * 0.5f;

	Vec3 point = Vec3(0.0f, 400.0f, 0.0f);
	mMainCamera.SetParams(((float)gScreenWidth) / gScreenHeight, 0.1f, 10000.0f, 1.0f, Vec3(0.0f, -1.0f, 0.0f), point, Vec3(0, 0, 1));
}

void World::AddDefaultDirectionalLight(int index)
{
	mLights[index].mDirection = glm::normalize(Vec3(1, 1, 1));
	mLights[index].mDiffuse = Vec3(1, 1, 1);
	mLights[index].mSpecular = Vec3(1, 1, 1);
	mLights[index].mAmbient = Vec3(1, 1, 1);
	mLights[index].mType = LightType::LT_DirectionalLight;
}

Light World::GetLight(int index)
{
	return mLights[index];
}

Matrix World::GetViewMatrix()
{
	Matrix view, proj;
	mMainCamera.GetViewTransformMatrix(view, proj);
	return view;
}

Matrix World::GetProjMatrix()
{
	Matrix view, proj;
	mMainCamera.GetViewTransformMatrix(view, proj);
	return proj;
}

void World::Update()
{
	static float theta = 0.0f;
	Vec3 point = Vec3(300.0f, 600.0f, 0.0f);
	mMainCamera.SetParams(((float)gScreenWidth) / gScreenHeight, 0.1f, 100000.0f, 1.0f, Vec3(sin(theta), 0.0f, cos(theta)), point, Vec3(0, -1, 0));
	theta += 0.01f;
}
