#pragma once
#include "Prefix.h"
#include "StaticLighting.h"

class DistanceFieldRasterPolicy
{
public:
	typedef StaticLightingVertex InterpolantType;

	DistanceFieldRasterPolicy(int InUpsampleFactor, int InSizeX, int InSizeY) :
		UpsampleFactor(InUpsampleFactor),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{}

protected:
	int GetMinX() const { return 0; }
	int GetMaxX() const { return SizeX - 1; }
	int GetMinY() const { return 0; }
	int GetMaxY() const { return SizeY - 1; }
	void ProcessPixel(int X, int Y, const InterpolantType& Interpolant);

private:
	const int UpsampleFactor;
	const int SizeX;
	const int SizeY;
};

void DistanceFieldRasterPolicy::ProcessPixel(int X, int Y, const InterpolantType& Interpolant)
{

}

template<class RasterPolicyType>
class TriangleRasterizer : public RasterPolicyType
{
public:
	TriangleRasterizer(const RasterPolicyType& InRasterPolicy) : RasterPolicyType(InRasterPolicy) {}

	typedef typename RasterPolicyType::InterpolantType InterpolantType;

	void DrawTriangle(
		const InterpolantType& v0, 
		const InterpolantType& v1, 
		const InterpolantType& v2,
		const Vec2& uv0,
		const Vec2& uv1,
		const Vec2& uv2)
	{
		InterpolantType Interpolants[3] = { v0, v1, v2 };
		Vec2 Uvs[3] = { uv0, uv1, uv2 };
		if (Uvs[0].y > Uvs[1].y)
		{
			std::swap(Uvs[0], Uvs[1]);
			std::swap(Interpolants[0], Interpolants[1]);
		}
		if (Uvs[0].y > Uvs[2].y)
		{
			std::swap(Uvs[0], Uvs[2]);
			std::swap(Interpolants[0], Interpolants[2]);
		}
		if (Uvs[1].y > Uvs[2].y)
		{
			std::swap(Uvs[1], Uvs[2]);
			std::swap(Interpolants[1], Interpolants[2]);
		}

		float TopTrapMinDiffX = (Uvs[1].x - Uvs[0].x) / (Uvs[1].y - Uvs[0].y);
		float TopTrapMaxDiffX = (Uvs[2].x - Uvs[0].x) / (Uvs[2].y - Uvs[0].y);
		InterpolantType TopTrapMinDiffInterpolant = (Interpolants[1] - Interpolants[0]) / (Uvs[1].y - Uvs[0].y);
		InterpolantType TopTrapMaxDiffInterpolant = (Interpolants[2] - Interpolants[0]) / (Uvs[2].y - Uvs[0].y);

		float	BottomTrapMinDiffX = (Uvs[2].x - Uvs[1].x) / (Uvs[2].y - Uvs[1].y);
		float BottomTrapMaxDiffX = (Uvs[2].x - Uvs[0].x) / (Uvs[2].y - Uvs[0].y);
		InterpolantType BottomTrapMinDiffInterpolant = (Interpolants[2] - Interpolants[1]) / (Uvs[2].y - Uvs[1].y);
		InterpolantType BottomTrapMaxDiffInterpolant = (Interpolants[2] - Interpolants[0]) / (Uvs[2].y - Uvs[0].y);

		DrawTrapeZoid(
			Interpolants[0],
			TopTrapMinDiffInterpolant,
			Interpolants[0],
			TopTrapMaxDiffInterpolant,
			Uvs[0].x,
			TopTrapMinDiffX,
			Uvs[0].x,
			TopTrapMaxDiffX,
			Uvs[0].y,
			Uvs[1].y
		);

		DrawTrapeZoid(
			Interpolants[1],
			BottomTrapMinDiffInterpolant,
			Interpolants[0] + TopTrapMaxDiffInterpolant * (Uvs[1].y - Uvs[0].y),
			BottomTrapMaxDiffInterpolant,
			Uvs[1].x,
			BottomTrapMinDiffX,
			Uvs[0].x + TopTrapMaxDiffX * (Uvs[1].y - Uvs[0].y),
			BottomTrapMaxDiffX,
			Uvs[1].y,
			Uvs[2].y
		);
	}


	void DrawTrapeZoid(
		const InterpolantType& TopMinInterpolant,
		const InterpolantType& DeltaMinInterpolant,
		const InterpolantType& TopMaxInterpolant,
		const InterpolantType& DeltaMaxInterpolant,
		float TopMinX,
		float DeltaMinX,
		float TopMaxX,
		float DeltaMaxX,
		float MinY,
		float MaxY)
	{
		int IntMinY = glm::clamp((int)std::ceil(MinY), RasterPolicyType::GetMinY(), RasterPolicyType::GetMaxY() + 1);
		int IntMaxY = glm::clamp((int)std::ceil(MaxY), RasterPolicyType::GetMinY(), RasterPolicyType::GetMaxY() + 1);
		for (int IntY = IntMinY; IntY < IntMaxY; IntY++)
		{
			float Y = IntY - MinY;
			float MinX = TopMinX + DeltaMinX * Y;
			float MaxX = TopMaxX + DeltaMaxX * Y;
			InterpolantType MinInterpolant = TopMinInterpolant + DeltaMinInterpolant * Y;
			InterpolantType MaxInterpolant = TopMaxInterpolant + DeltaMaxInterpolant * Y;
			if (MinX > MaxX)
			{
				std::swap(MinX, MaxX);
				std::swap(MinInterpolant, MaxInterpolant);
			}
			if (MaxX > MinX)
			{
				int IntMinX = glm::clamp((int)std::ceil(MinX), RasterPolicyType::GetMinX(), RasterPolicyType::GetMaxX() + 1);
				int IntMaxX = glm::clamp((int)std::ceil(MaxX), RasterPolicyType::GetMinX(), RasterPolicyType::GetMaxX() + 1);
				InterpolantType DeltaInterpolant = (MaxInterpolant - MinInterpolant) / (MaxX - MinX);
				for (int X = IntMinX; X < IntMaxX; X++)
				{
					RasterPolicyType::ProcessPixel(X, IntY, MinInterpolant + DeltaInterpolant * (X - MinX));
				}
			}
		}
	}
};