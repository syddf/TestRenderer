#pragma once

#include "Prefix.h"

struct StaticLightingVertex
{
	Point Position;
	Vec3 Normal;
	Vec2 TexCoord0;
	Vec2 TexCoord1;

	friend StaticLightingVertex operator-(const StaticLightingVertex& V0, const StaticLightingVertex& V1)
	{
		StaticLightingVertex Result;
		Result.Position = V0.Position - V1.Position;
		Result.Normal = V0.Normal - V1.Normal;
		Result.TexCoord0 = V0.TexCoord0 - V1.TexCoord0;
		Result.TexCoord1 = V0.TexCoord1 - V1.TexCoord1;
		return Result;
	}

	friend StaticLightingVertex operator+(const StaticLightingVertex& V0, const StaticLightingVertex& V1)
	{
		StaticLightingVertex Result;
		Result.Position = V0.Position + V1.Position;
		Result.Normal = V0.Normal + V1.Normal;
		Result.TexCoord0 = V0.TexCoord0 + V1.TexCoord0;
		Result.TexCoord1 = V0.TexCoord1 + V1.TexCoord1;
		return Result;
	}

	friend StaticLightingVertex operator*(const StaticLightingVertex& V0, float scale)
	{
		StaticLightingVertex Result;
		Result.Position = V0.Position * scale;
		Result.Normal = V0.Normal * scale;
		Result.TexCoord0 = V0.TexCoord0 * scale;
		Result.TexCoord1 = V0.TexCoord1 * scale;
		return Result;
	}

	friend StaticLightingVertex operator/(const StaticLightingVertex& V0, float scale)
	{
		StaticLightingVertex Result;
		float inv = 1.0f / scale;
		Result.Position = V0.Position * inv;
		Result.Normal = V0.Normal * inv;
		Result.TexCoord0 = V0.TexCoord0 * inv;
		Result.TexCoord1 = V0.TexCoord1 * inv;
		return Result;
	}
};