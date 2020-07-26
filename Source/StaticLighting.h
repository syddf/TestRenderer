#pragma once

#include "Prefix.h"
#include "Geometry.h"
#include "Rasterize.h"
#include "StaticLightingData.h"

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

	void RunStaticLighting(const std::vector<StaticLightingMesh>& Mesh, UInt32 Width, UInt32 Height)
	{
		std::vector<TexelToVertexMap> TexelToVertexMapVec;
		std::vector<TexelToCornersMap> TexelToCornersMapVec;
		size_t MeshCount = Mesh.size();
		TexelToVertexMapVec.reserve(MeshCount);
		TexelToCornersMapVec.reserve(MeshCount);

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
					StaticLightingVertex V0 = VertexVec[VertIndex];
					StaticLightingVertex V1 = VertexVec[VertIndex + 1];
					StaticLightingVertex V2 = VertexVec[VertIndex + 2];
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

			}

			int UpsampleFactor = 1;
			if (AverageTexelDensity > EPSILON)
			{
				AverageTexelDensity /= (VertexVec.size() / 3);
				const float RightTriangleSide = std::sqrt(2.0f * AverageTexelDensity);
				const float ApproximateHighResTexelsPerMaxTransitionDistance = 6.0f;
				const float MaxTransitionDistanceWorldSpace = 50.0f;
				int MinDistanceFieldUpsampleFactor = 5;
				const int TargetUpsampleFactor = std::floor(ApproximateHighResTexelsPerMaxTransitionDistance / (RightTriangleSide * MaxTransitionDistanceWorldSpace));
				UpsampleFactor = glm::clamp(TargetUpsampleFactor - TargetUpsampleFactor % 2 + 1, MinDistanceFieldUpsampleFactor, 13);
			}

			// Phase 1 : Low Resolution Visibility

		}
	}
};