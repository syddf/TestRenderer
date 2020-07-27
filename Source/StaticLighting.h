#pragma once

#include "Prefix.h"
#include "Geometry.h"
#include "Rasterize.h"
#include "StaticLightingData.h"
#include "Light.h"
#include "BVH.h"

struct VisibilitySample
{
protected:
	Vec4 PositionAndOccluderDistance;
	Vec3 Normal;
	UInt32 Visible : 1;
	UInt32 IsMapped : 1;

public:
	VisibilitySample()
	{
		Visible = false;
		IsMapped = false;
	}

	void SetPosition(const Vec3& Position)
	{
		PositionAndOccluderDistance.x = Position.x;
		PositionAndOccluderDistance.y = Position.y;
		PositionAndOccluderDistance.z = Position.z;
	}
	void SetOccluderDistance(float Distance)
	{
		PositionAndOccluderDistance.w = Distance;
	}
	void SetNormal(const Vec3& Norm)
	{
		Normal = Norm;
	}
	void SetVisible(bool bVisible)
	{
		Visible = bVisible;
	}
	void SetMapped(bool bMapped)
	{
		IsMapped = bMapped;
	}

	Vec3 GetPosition() const
	{
		return Vec3(PositionAndOccluderDistance.x, PositionAndOccluderDistance.y, PositionAndOccluderDistance.z);
	}

	float GetOccluderDistance() const
	{
		return PositionAndOccluderDistance.w;
	}

	Vec3 GetNormal() const
	{
		return Normal;
	}

	bool GetMapped() const
	{
		return IsMapped;
	}

	bool GetVisible() const
	{
		return Visible;
	}
};

struct LowResolutionVisibilitySample : public VisibilitySample
{
	using HighResolutionSampleVec = std::vector<VisibilitySample>;
	UInt32 NeedHighResolutionSamples : 1;
	HighResolutionSampleVec HighResolutionSamples;

	inline void SetNeedsHighResSampling(bool NeedHighResolution, int UpSampleFactor)
	{
		if (NeedHighResolution)
		{
			HighResolutionSamples.resize(UpSampleFactor * UpSampleFactor);
		}
	}
};

struct TexelVisibilityData2D
{
public:
	TexelVisibilityData2D(UInt32 InSizeX, UInt32 InSizeY)
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		Data.resize(InSizeX * InSizeY);
	}

public:
	UInt32 SizeX;
	UInt32 SizeY;
	using VisibilityVec = std::vector<LowResolutionVisibilitySample>;
	VisibilityVec Data;

	LowResolutionVisibilitySample& operator()(UInt32 X, UInt32 Y)
	{
		UInt32 Index = Y * SizeX + X;
		return Data[Index];
	}
};

struct TexelToVertex
{
	Point Position;
	Vec3 Normal;
	Vec2 TexCoord0;
	Vec2 TexCoord1;
	float TotalSampleWeight;
	float MaxSampleWeight;

	StaticLightingVertex GetVertex() const 
	{
		StaticLightingVertex Vertex;
		Vertex.Position = Position;
		Vertex.Normal = Normal;
		Vertex.TexCoord0 = TexCoord0;
		Vertex.TexCoord1 = TexCoord1;
		return Vertex;
	}
};

struct TexelToVertexMap
{
	TexelToVertexMap()
	{

	}

	void Resize(UInt32 InSizeX, UInt32 InSizeY)
	{
		SizeX = InSizeX;
		SizeY = InSizeY;
		Data.resize(InSizeX * InSizeY);
	}

	TexelToVertexMap(UInt32 InSizeX, UInt32 InSizeY) :
		SizeX(InSizeX), SizeY(InSizeY)
	{
		Data.resize(InSizeX * InSizeY);
	}

	TexelToVertex& operator()(UInt32 X, UInt32 Y)
	{
		UInt32 Index = Y * SizeX + X;
		return Data[Index];
	}

	using TexelVec = std::vector<TexelToVertex>;
	UInt32 SizeX;
	UInt32 SizeY;
	TexelVec Data;
};

struct TexelToCorners
{
	Vec3 Position[4];
	bool Valid[4];
};

struct TexelToCornersMap
{
	TexelToCornersMap()
	{

	}

	void Resize(UInt32 InSizeX, UInt32 InSizeY)
	{
		SizeX = InSizeX;
		SizeY = InSizeY;
		Data.resize(InSizeX * InSizeY);
	}


	TexelToCornersMap(UInt32 InSizeX, UInt32 InSizeY)
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		Data.resize(SizeX * SizeY);
	}

	TexelToCorners& operator()(UInt32 X, UInt32 Y)
	{
		UInt32 Index = Y * SizeX + X;
		return Data[Index];
	}

	using CornersVec = std::vector<TexelToCorners>;
	UInt32 SizeX;
	UInt32 SizeY;
	CornersVec Data;
};

class TexelCornerRasterPolicy
{
public:
	typedef StaticLightingVertex InterpolantType;

	TexelCornerRasterPolicy(TexelToCornersMap& InTexelToCornersMap, int InCornerIndex)
		:TexelToCornersMap(InTexelToCornersMap), CornerIndex(InCornerIndex)
	{
		
	}

protected:
	int GetMinX() const { return 0; }
	int GetMaxX() const { return TexelToCornersMap.SizeX - 1; }
	int GetMinY() const { return 0; }
	int GetMaxY() const { return TexelToCornersMap.SizeY - 1; }
	void ProcessPixel(int X, int Y, const InterpolantType& Interpolant)
	{
		TexelToCorners& TexelToCorners = TexelToCornersMap(X, Y);
		TexelToCorners.Position[CornerIndex] = Interpolant.Position;
		TexelToCorners.Valid[CornerIndex] = true;
	}

public:
	TexelToCornersMap& TexelToCornersMap;
	const int CornerIndex;
};

struct SignedDistanceFieldShadowSampleData
{
	float Distance;
	float PenumbraSize;
	bool IsMapped;
};

struct SignedDistanceFieldShadowMapData2D
{
	SignedDistanceFieldShadowMapData2D(UInt32 InSizeX, UInt32 InSizeY)
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		Data.resize(SizeX * SizeY);
	}

	SignedDistanceFieldShadowSampleData& operator()(UInt32 X, UInt32 Y) { return Data[SizeX * Y + X]; }

	using SDFShadowDataVec = std::vector<SignedDistanceFieldShadowSampleData>;
	SDFShadowDataVec Data;

	UInt32 SizeX;
	UInt32 SizeY;
};


class StaticLightingRasterPolicy
{
public:
	typedef StaticLightingVertex InterpolantType;

	StaticLightingRasterPolicy(
		TexelToVertexMap& InTexelToVertexMap,
		float InSampleWeight,
		Vec3 InTriangleNormal)
		:TexelToVertexMap(InTexelToVertexMap), SampleWeight(InSampleWeight), TriangleNormal(InTriangleNormal)
	{

	}
	TexelToVertexMap& TexelToVertexMap;
	const Vec3 TriangleNormal;
	const float SampleWeight;

protected:
	int GetMinX() const { return 0; }
	int GetMaxX() const { return TexelToVertexMap.SizeX - 1; }
	int GetMinY() const { return 0; }
	int GetMaxY() const { return TexelToVertexMap.SizeY - 1; }

	void ProcessPixel(int X, int Y, const InterpolantType& Interpolant)
	{
		TexelToVertex& TexelToVertex = TexelToVertexMap(X, Y);
		if (SampleWeight > TexelToVertex.MaxSampleWeight)
		{
			TexelToVertex.MaxSampleWeight = SampleWeight;
			TexelToVertex.Position = Interpolant.Position;
			TexelToVertex.TexCoord0 = Interpolant.TexCoord0;
			TexelToVertex.TexCoord1 = Interpolant.TexCoord1;
			TexelToVertex.Normal += TriangleNormal * SampleWeight;
			TexelToVertex.TotalSampleWeight += SampleWeight;
		}
	}
};

class DistanceFieldRasterPolicy
{
public:
	typedef StaticLightingVertex InterpolantType;

	DistanceFieldRasterPolicy(TexelVisibilityData2D& InLowResolutionVisibilityData, int InUpsampleFactor, int InSizeX, int InSizeY) :
		LowResolutionVisibilityData(InLowResolutionVisibilityData),
		UpsampleFactor(InUpsampleFactor),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
	
	}

protected:
	int GetMinX() const { return 0; }
	int GetMaxX() const { return SizeX - 1; }
	int GetMinY() const { return 0; }
	int GetMaxY() const { return SizeY - 1; }

	void ProcessPixel(int X, int Y, const InterpolantType& Interpolant)
	{
		LowResolutionVisibilitySample& LowResSample = LowResolutionVisibilityData(X / UpsampleFactor, Y / UpsampleFactor);
		if (LowResSample.NeedHighResolutionSamples)
		{
			VisibilitySample& Sample = LowResSample.HighResolutionSamples[Y % UpsampleFactor * UpsampleFactor + X % UpsampleFactor];
			Sample.SetPosition(Interpolant.Position);
			Sample.SetNormal(Interpolant.Normal);
			Sample.SetMapped(true);
		}
	}

protected:
	TexelVisibilityData2D& LowResolutionVisibilityData;
	const int UpsampleFactor;
	const int SizeX;
	const int SizeY;

};

struct StaticLightingSystem
{
	using TriangleVec = std::vector<Triangle>;
	void CalcTexelCorners(std::vector<StaticLightingVertex>& VertexVec, TexelToCornersMap& TexelToCornersMap)
	{
		const Vec2 CornerOffsets[4] =
		{
			Vec2(0, 0),
			Vec2(-1, 0),
			Vec2(0, -1),
			Vec2(-1, -1)
		};

		for (size_t Index = 0; Index < VertexVec.size();)
		{
			StaticLightingVertex V0 = VertexVec[Index];
			StaticLightingVertex V1 = VertexVec[Index + 1];
			StaticLightingVertex V2 = VertexVec[Index + 2];
			
			for (int CornerIndex = 0; CornerIndex < 4; CornerIndex++)
			{
				TriangleRasterizer<TexelCornerRasterPolicy> TexelCornerRas(TexelCornerRasterPolicy(TexelToCornersMap, CornerIndex));
				TexelCornerRas.DrawTriangle(
					V0, 
					V1, 
					V2, 
					V0.TexCoord1 * Vec2(TexelToCornersMap.SizeX, TexelToCornersMap.SizeY) + CornerOffsets[CornerIndex],
					V1.TexCoord1 * Vec2(TexelToCornersMap.SizeX, TexelToCornersMap.SizeY) + CornerOffsets[CornerIndex], 
					V2.TexCoord1 * Vec2(TexelToCornersMap.SizeX, TexelToCornersMap.SizeY) + CornerOffsets[CornerIndex]);
			}
			Index += 3;
		}
	}

	void SetupTextureMapping(std::vector<StaticLightingVertex>& VertexVec, TexelToVertexMap& TexelToVertexMap, TexelToCornersMap& TexelToCornersMap)
	{
		CalcTexelCorners(VertexVec, TexelToCornersMap);
		for (size_t Index = 0; Index < VertexVec.size();)
		{
			StaticLightingVertex V0 = VertexVec[Index];
			StaticLightingVertex V1 = VertexVec[Index + 1];
			StaticLightingVertex V2 = VertexVec[Index + 2];
			Vec3 TriangleNormal = glm::cross(V2.Position - V0.Position, V1.Position - V0.Position);
			if (TriangleNormal.length() > EPSILON)
			{
				const Vec2 UV0 = V0.TexCoord1 * Vec2(TexelToVertexMap.SizeX, TexelToVertexMap.SizeY);
				const Vec2 UV1 = V1.TexCoord1 * Vec2(TexelToVertexMap.SizeX, TexelToVertexMap.SizeY);
				const Vec2 UV2 = V2.TexCoord1 * Vec2(TexelToVertexMap.SizeX, TexelToVertexMap.SizeY);

				const UInt32 NumSamplesX = 7;
				const UInt32 NumSamplesY = 7;

				for (int Y = 1; Y < NumSamplesY - 1; Y++)
				{
					const float SampleYOffset = -Y / (float)(NumSamplesY - 1);
					for (int X = 1; X < NumSamplesX - 1; X++)
					{
						const float SampleXOffset = -X / (float)(NumSamplesX - 1);
						const float SampleWeight = (1.0f - std::abs(1 + SampleXOffset * 2)) * (1.0f - std::abs(1 + SampleYOffset * 2));\

						TriangleRasterizer<StaticLightingRasterPolicy> TexelMappingRasterizer(StaticLightingRasterPolicy(
								TexelToVertexMap,
								SampleWeight,
								TriangleNormal
							));

						TexelMappingRasterizer.DrawTriangle(
							V0,
							V1,
							V2,
							UV0 + Vec2(SampleXOffset, SampleYOffset),
							UV1 + Vec2(SampleXOffset, SampleYOffset),
							UV2 + Vec2(SampleXOffset, SampleYOffset)
						);
					}
				}
				
			}
		}
	}

	void RunStaticLighting(std::vector<StaticLightingMesh>& Mesh, UInt32 Width, UInt32 Height, DirectionalLight DirectionLight)
	{
		auto InRange = [&](int X, int Y, int Width, int Height)-> bool
		{
			return X > 0 && X < Width && Y > 0 && Y < Height;
		};

		const int MaxTransitionDistanceWorldSpace = 50;

		std::vector<TexelToVertexMap> TexelToVertexMapVec;
		std::vector<TexelToCornersMap> TexelToCornersMapVec;
		size_t MeshCount = Mesh.size();
		TexelToVertexMapVec.reserve(MeshCount);
		TexelToCornersMapVec.reserve(MeshCount);

		std::vector<Triangle> BVHTriangles;
		for (auto& StaticMesh : Mesh)
		{
			std::vector<Triangle> Triangles;
			StaticMesh.GetTriangleVec(Triangles);
			BVHTriangles.insert(BVHTriangles.end(), Triangles.begin(), Triangles.end());
		}
		BVHTree BVHTree;
		BVHTree.BuildBVHTree(BVHTriangles);

		for (size_t Index = 0; Index < MeshCount; Index++)
		{
			// Pre : Get Map / UpSampleFactor
			const StaticLightingMesh& CurMesh = Mesh[Index];
			TexelToVertexMap& TexelToVertexMap = TexelToVertexMapVec[Index];
			TexelToVertexMap.Resize(Width, Height);
			TexelToCornersMap& TexelToCornersMap = TexelToCornersMapVec[Index];
			TexelToCornersMap.Resize(Width, Height);
			std::vector<StaticLightingVertex> VertexVec;
			CurMesh.GetStaticLightingVertexVec(VertexVec);

			SetupTextureMapping(VertexVec, TexelToVertexMap, TexelToCornersMap);

			float AverageTexelDensity = 0.0f;
			int UpSampleFactor = 1;
			for (int VertIndex = 0; VertIndex < VertexVec.size(); )
			{
				StaticLightingVertex& V0 = VertexVec[VertIndex];
				StaticLightingVertex& V1 = VertexVec[VertIndex + 1];
				StaticLightingVertex& V2 = VertexVec[VertIndex + 2];
				Vec3 Normal = glm::cross((V2.Position - V0.Position), (V1.Position - V0.Position));
				float TriangleArea = 0.5f * Normal.length();

				if (TriangleArea > EPSILON)
				{
					const Vec2 Vertex0 = VertexVec[VertIndex].TexCoord1 * Vec2(Width, Height);
					const Vec2 Vertex1 = VertexVec[VertIndex + 1].TexCoord1* Vec2(Width, Height);
					const Vec2 Vertex2 = VertexVec[VertIndex + 2].TexCoord1 * Vec2(Width, Height);

					// Area in lightmap space, or the number of lightmap texels covered by this triangle
					const float LightmapTriangleArea = std::abs(
						Vertex0.x * (Vertex1.y - Vertex2.y)
						+ Vertex1.x * (Vertex2.y - Vertex0.y)
						+ Vertex2.x * (Vertex0.y - Vertex1.y));

					// Accumulate the texel density
					AverageTexelDensity += LightmapTriangleArea / TriangleArea;
				}

				VertIndex += 3;
			}

			if (AverageTexelDensity > EPSILON)
			{
				AverageTexelDensity /= (VertexVec.size() / 3);
				const float RightTriangleSide = std::sqrt(2.0f * AverageTexelDensity);
				const float ApproximateHighResTexelsPerMaxTransitionDistance = 6.0f;
				const float MaxTransitionDistanceWorldSpace = 50.0f;
				int MinDistanceFieldUpsampleFactor = 5;
				const int TargetUpsampleFactor = std::floor(ApproximateHighResTexelsPerMaxTransitionDistance / (RightTriangleSide * MaxTransitionDistanceWorldSpace));
				UpSampleFactor = glm::clamp(TargetUpsampleFactor - TargetUpsampleFactor % 2 + 1, MinDistanceFieldUpsampleFactor, 13);
			}

			// Phase 1 : Low Resolution Visibility
			TexelVisibilityData2D Visibility2D(Width, Height);
			for (int Y = 0; Y < Height; Y++)
			{
				for (int X = 0; X < Width; X++)
				{
					TexelToVertex& TexelToVertex = TexelToVertexMap(X, Y);
					if (TexelToVertex.TotalSampleWeight > 0)
					{
						LowResolutionVisibilitySample& CurrentSample = Visibility2D(X, Y);
						CurrentSample.SetPosition(TexelToVertex.Position);
						CurrentSample.SetNormal(TexelToVertex.Normal);
						CurrentSample.SetMapped(true);

						const Vec3 LightPosition = DirectionLight.Position;
						const Vec3 LightVec = DirectionLight.Direction;
						Ray LightRay;
						LightRay.Origin = TexelToVertex.Position;
						LightRay.Direction = glm::normalize(LightPosition - LightRay.Origin);
						Intersection Intersection;
						if (BVHTree.Intersect(LightRay, Intersection))
						{
							CurrentSample.SetVisible(true);
						}
					}
				}
			}

			// Phase 2 : Detecting Additional Sampling Required¡¢Rasterize High Resolution Samples
			int HighResolutionX = Width * UpSampleFactor;
			int HighResolutionY = Height * UpSampleFactor;

			SignedDistanceFieldShadowMapData2D* ShadowMapData = new SignedDistanceFieldShadowMapData2D(Width, Height);
			const int Neighbors[4][2] = 
			{
				{0, 1},
				{0, -1},
				{1, 0},
				{-1, 0}
			};

			const int Corners[4][2] = 
			{
				{0, 0},
			    {0, UpSampleFactor - 1},
				{UpSampleFactor - 1, 0},
				{UpSampleFactor - 1, UpSampleFactor - 1}
			};

			for (int Y = 0; Y < Height; Y++)
			{
				for (int X = 0; X < Width; X++)
				{
					LowResolutionVisibilitySample& CurrentSample = Visibility2D(X, Y);
					if (CurrentSample.GetMapped())
					{
						SignedDistanceFieldShadowSampleData& FinalShadowSample = (*ShadowMapData)(X, Y);
						FinalShadowSample.IsMapped = true;
						if (CurrentSample.GetVisible())
						{
							FinalShadowSample.Distance = 1.0f;
							FinalShadowSample.PenumbraSize = 1.0f;
						}
						bool NeighborDifferent = false;
						for (int i = 0; i < 4; i++)
						{
							int NewX = X + Neighbors[i][0];
							int NewY = Y + Neighbors[i][1];
							if (InRange(NewX, NewY, Width, Height))
							{
								const LowResolutionVisibilitySample& NeighborSample = Visibility2D(NewX, NewY);
								if (CurrentSample.GetVisible() != NeighborSample.GetVisible() && NeighborSample.GetMapped())
								{
									NeighborDifferent = true;
									break;
								}
							}
						}

						if (NeighborDifferent)
						{
							CurrentSample.SetNeedsHighResSampling(true, UpSampleFactor);
						}
					}
				}
			}

			DistanceFieldRasterPolicy RasterPolicy(Visibility2D, UpSampleFactor, HighResolutionX, HighResolutionY);
			TriangleRasterizer<DistanceFieldRasterPolicy> DistanceFieldRasterizer(RasterPolicy);
			for (int VertIndex = 0; VertIndex < VertexVec.size();)
			{
				StaticLightingVertex& V0 = VertexVec[VertIndex];
				StaticLightingVertex& V1 = VertexVec[VertIndex + 1];
				StaticLightingVertex& V2 = VertexVec[VertIndex + 2];

				DistanceFieldRasterizer.DrawTriangle(
					V0,
					V1,
					V2,
					V0.TexCoord1 * Vec2(HighResolutionX, HighResolutionY) + Vec2(-0.5f, -0.5f),
					V1.TexCoord1 * Vec2(HighResolutionX, HighResolutionY) + Vec2(-0.5f, -0.5f),
					V2.TexCoord1* Vec2(HighResolutionX, HighResolutionY) + Vec2(-0.5f, -0.5f)
				);
			}

			// Edge Case : Low Resolution Sample Is Mapped , No High Resolution Sample Is Mapped
			for (int Y = 0; Y < Height; Y++)
			{
				for (int X = 0; X < Width; X++)
				{
					LowResolutionVisibilitySample& CurrentSample = Visibility2D(X, Y);
					if (CurrentSample.GetMapped() && CurrentSample.NeedHighResolutionSamples)
					{
						bool HasHighResSampleMapped = false;
						for (int HighResY = 0; HighResY < UpSampleFactor; HighResY++)
						{
							for (int HighResX = 0; HighResX < UpSampleFactor; HighResX++)
							{
								VisibilitySample& CurrentHighResSample = CurrentSample.HighResolutionSamples[HighResY * UpSampleFactor + HighResX];
								if (CurrentHighResSample.GetMapped())
								{
									HasHighResSampleMapped = true;
								}
							}
						}

						if (!HasHighResSampleMapped)
						{
							const TexelToCorners& TexelToCorners = TexelToCornersMap(X, Y);
							for (int CornerIndex = 0; CornerIndex < 4; CornerIndex++)
							{
								if (TexelToCorners.Valid[CornerIndex])
								{
									VisibilitySample& CornerHighResSample = CurrentSample.HighResolutionSamples[Corners[CornerIndex][1] * UpSampleFactor + Corners[CornerIndex][0]];
									CornerHighResSample.SetMapped(true);
									CornerHighResSample.SetPosition(TexelToCorners.Position[CornerIndex]);
								}
							}
						}
					}
				}
			}

			for (int Y = 0; Y < Height; Y++)
			{
				for (int X = 0; X < Width; X++)
				{
					LowResolutionVisibilitySample& CurrentSample = Visibility2D(X, Y);
					if (CurrentSample.GetMapped() && CurrentSample.NeedHighResolutionSamples)
					{
						for (int HighResY = 0; HighResY < UpSampleFactor; HighResY++)
						{
							for (int HighResX = 0; HighResX < UpSampleFactor; HighResX++)
							{
								VisibilitySample& HighResSample = CurrentSample.HighResolutionSamples[HighResY * UpSampleFactor + HighResX];
								if (true) // Test
								{
									Vec3 Position = DirectionLight.Position;
									Vec3 Direction = glm::normalize(DirectionLight.Position - HighResSample.GetPosition());
									Ray ray;
									ray.Origin = Position;
									ray.Direction = Direction;

									Intersection Inter;
									if (BVHTree.Intersect(ray, Inter))
									{
										HighResSample.SetOccluderDistance(Inter.T);
									}
									else
									{
										HighResSample.SetVisible(true);
									}
								}
							}
						}
					}
				}
			}


			// Phase 3 : Traverse High Resolution To Get Shadow Transition And Scatter To Other Samples.
			for (int LowResY = 0; LowResY < Height; LowResY++)
			{
				for (int LowResX = 0; LowResX < Width; LowResX++)
				{
					LowResolutionVisibilitySample& CurrentLowResSample = Visibility2D(LowResX, LowResY);
					if (CurrentLowResSample.GetMapped() && CurrentLowResSample.NeedHighResolutionSamples)
					{
						for (int HighResY = 0; HighResY < UpSampleFactor; HighResY++)
						{
							for (int HighResX = 0; HighResX < UpSampleFactor; HighResX++)
							{
								VisibilitySample& HighResSample = CurrentLowResSample.HighResolutionSamples[HighResY * UpSampleFactor + HighResX];
								if (HighResSample.GetMapped() && !HighResSample.GetVisible())
								{
									bool NeighborDifferent = false;
									for (int i = 0; i < 4; i++)
									{
										int HighResNewX = LowResX * UpSampleFactor + HighResX + Neighbors[i][0];
										int HighResNewY = LowResY * UpSampleFactor + HighResY + Neighbors[i][1];
										int LowResNewX = HighResNewX / UpSampleFactor;
										int LowResNewY = HighResNewY / UpSampleFactor;
										if (InRange(LowResNewX, LowResNewY, Width, Height))
										{
											const LowResolutionVisibilitySample& LowResNeighborSample = Visibility2D(LowResNewX, LowResNewY);
											if (LowResNeighborSample.NeedHighResolutionSamples)
											{
												const VisibilitySample& HighResNeighborSample = 
													LowResNeighborSample.HighResolutionSamples[(HighResNewY) % UpSampleFactor * UpSampleFactor + HighResNewX % UpSampleFactor];
												if (HighResNeighborSample.GetMapped() && HighResNeighborSample.GetVisible())
												{
													NeighborDifferent = true;
													break;
												}
											}
											else
											{
												if (LowResNeighborSample.GetMapped() && LowResNeighborSample.GetVisible())
												{
													NeighborDifferent = true;
													break;
												}
											}
										}
									}

									if (NeighborDifferent)
									{
										float WorldSpacePerHighResTexelX = MAXF;
										float WorldSpacePerHighResTexelY = MAXF;

										for (int NeighborIndex = 0; NeighborIndex < 4; NeighborIndex++)
										{
											int NewHighResX = HighResX + Neighbors[NeighborIndex][0];
											int NewHighResY = HighResY + Neighbors[NeighborIndex][1];
											if (InRange(NewHighResX, NewHighResY, UpSampleFactor, UpSampleFactor))
											{
												const VisibilitySample& NeighborSample = 
													CurrentLowResSample.HighResolutionSamples[NewHighResY * UpSampleFactor + NewHighResX];
												float Length = (NeighborSample.GetPosition() - HighResSample.GetPosition()).length();
												if (NeighborIndex >= 2)
												{
													WorldSpacePerHighResTexelX = std::min(WorldSpacePerHighResTexelX, Length);
												}
												else
												{
													WorldSpacePerHighResTexelY = std::min(WorldSpacePerHighResTexelY, Length);
												}
											}
										}

										if (WorldSpacePerHighResTexelX == MAXF && WorldSpacePerHighResTexelY == MAXF)
										{
											WorldSpacePerHighResTexelX = 1.0f;
											WorldSpacePerHighResTexelY = 1.0f;
										}
										else if (WorldSpacePerHighResTexelX == MAXF)
										{
											WorldSpacePerHighResTexelX = WorldSpacePerHighResTexelY;
										}
										else if (WorldSpacePerHighResTexelY == MAXF) 
										{
											WorldSpacePerHighResTexelY = WorldSpacePerHighResTexelX;
										}

										int LowResScatterTexelsCountX = (int)std::floor(MaxTransitionDistanceWorldSpace / (WorldSpacePerHighResTexelX * UpSampleFactor)) + 1;
										LowResScatterTexelsCountX = std::min(LowResScatterTexelsCountX, 100);
										int LowResScatterTexelsCountY = (int)std::floor(MaxTransitionDistanceWorldSpace / (WorldSpacePerHighResTexelY * UpSampleFactor)) + 1;
										LowResScatterTexelsCountY = std::min(LowResScatterTexelsCountY, 100);


										for (int ScatterOffsetY = -LowResScatterTexelsCountY; ScatterOffsetY <= LowResScatterTexelsCountY; ScatterOffsetY++)
										{
											const int LowResScatterY = ScatterOffsetY + LowResY;
											if (LowResScatterY < 0 || LowResScatterY >= Height)
												continue;
											for (int ScatterOffsetX = -LowResScatterTexelsCountX; ScatterOffsetX <= LowResScatterTexelsCountX; ScatterOffsetX++)
											{
												const int LowResScatterX = ScatterOffsetX + LowResX;
												if (LowResScatterX < 0 || LowResScatterX >= Width)
												{
													continue;
												}

												const LowResolutionVisibilitySample& LowResScatterSample = Visibility2D(LowResScatterX, LowResScatterY);
												if (LowResScatterSample.GetMapped())
												{
													bool CurrentRegion = false;
													Vec3 ScatterPosition;
													Vec3 ScatterNormal;
													bool FoundScatterPosition = false;

													if (LowResScatterSample.NeedHighResolutionSamples)
													{
														int CenterSampleIndex = (UpSampleFactor / 2) * UpSampleFactor + UpSampleFactor / 2;
														const VisibilitySample& HighResScatterSample = LowResScatterSample.HighResolutionSamples[CenterSampleIndex];
														if (HighResScatterSample.GetMapped())
														{
															CurrentRegion = HighResScatterSample.GetVisible();
															ScatterPosition = HighResScatterSample.GetPosition();
															ScatterNormal = HighResScatterSample.GetNormal();
															FoundScatterPosition = true;
														}
														else
														{
															float ClosestMappedSubSampleDist = MAXF;
															for (int SubY = 0; SubY < UpSampleFactor; SubY++)
															{
																for (int SubX = 0; SubX < UpSampleFactor; SubX++)
																{
																	const VisibilitySample& SubHighResSample = LowResScatterSample.HighResolutionSamples[SubY * UpSampleFactor + SubX];
																	const float SubSampleDistanceSquared =
																		(SubX - UpSampleFactor / 2) * (SubX - UpSampleFactor / 2) + (SubY - UpSampleFactor / 2) * (SubY - UpSampleFactor / 2);
																	if (SubHighResSample.GetMapped() && SubSampleDistanceSquared < ClosestMappedSubSampleDist)
																	{
																		ClosestMappedSubSampleDist = SubSampleDistanceSquared;
																		CurrentRegion = SubHighResSample.GetVisible();
																		ScatterPosition = SubHighResSample.GetPosition();
																		ScatterNormal = SubHighResSample.GetNormal();
																		FoundScatterPosition = true;
																	}
																}
															}
														}
													}

													if (!FoundScatterPosition)
													{
														CurrentRegion = LowResScatterSample.GetVisible();
														ScatterPosition = LowResScatterSample.GetPosition();
														ScatterNormal = LowResScatterSample.GetNormal();
													}
													
													const float TransitionDistance = (ScatterPosition - HighResSample.GetPosition()).length();
													const float NormalizedDistance = glm::clamp(TransitionDistance / MaxTransitionDistanceWorldSpace, 0.0f , 1.0f);
													SignedDistanceFieldShadowSampleData& FinalShadowSample = (*ShadowMapData)(LowResScatterX, LowResScatterY);
													if (NormalizedDistance * 0.5f < std::abs(FinalShadowSample.Distance - 0.5f))
													{
														FinalShadowSample.Distance = CurrentRegion ? NormalizedDistance * 0.5f + 0.5f : -0.5f * NormalizedDistance + 0.5f;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
};