#pragma once
#include "Prefix.h"

struct Intersection
{
	Point Position;
	Vec3 Normal;
	float T;

public:
	Intersection()
	{
		T = MAXF;
	}
};

struct Ray
{
public:
    Point Origin;
    Vec3 Direction;
};

struct AABB
{
	Point Min = Point(MAXF, MAXF, MAXF);
	Point Max = Point(-MAXF, -MAXF, -MAXF);

	void Union(const AABB& aabb)
	{
		Min.x = std::min(aabb.Min.x, Min.x);
		Min.y = std::min(aabb.Min.y, Min.y);
		Min.z = std::min(aabb.Min.z, Min.z);

		Max.x = std::max(aabb.Max.x, Max.x);
		Max.y = std::max(aabb.Max.y, Max.y);
		Max.z = std::max(aabb.Max.z, Max.z);		
	}

	void Intersection(const AABB& aabb)
	{
		Min.x = std::max(aabb.Min.x, Min.x);
		Min.y = std::max(aabb.Min.y, Min.y);
		Min.z = std::max(aabb.Min.z, Min.z);

		Max.x = std::min(aabb.Max.x, Max.x);
		Max.y = std::min(aabb.Max.y, Max.y);
		Max.z = std::min(aabb.Max.z, Max.z);
	}

	bool Intersect(const Ray& ray) const
	{
		float TMin = 0.0f, TMax = MAXF;
		for (int i = 0; i < 3; i++)
		{
			if (std::fabsf(ray.Direction[i]) < EPSILON)
			{
				if (ray.Origin[i] < Min[i] || ray.Origin[i] > Max[i])
					return false;
			}
			else
			{
				const float InvD = 1.0f / ray.Direction[i];
				float T1 = (Min[i] - ray.Origin[i]) * InvD;
				float T2 = (Max[i] - ray.Origin[i]) * InvD;
				if (T1 > T2)
					std::swap(T1, T2);
				if (T1 > TMin)
					TMin = T1;
				if (T2 < TMax)
					TMax = T2;
				if (TMin > TMax)
					return false;
			}
		}
		return true;
	}

	float SurfaceArea() const
	{
		Vec3 Diag = Max - Min;
		return 2.0f * (Diag.x * Diag.y + Diag.x * Diag.z + Diag.y * Diag.z);
	}

	int GetMaxAxis() const
	{
		Vec3 Diag = Max - Min;
		if (Diag.x > Diag.y)
		{
			return Diag.x > Diag.z ? 0 : 2;
		}
		return Diag.y > Diag.z ? 1 : 2;
	}

	Point GetCentroid() const
	{
		return 0.5f * (Min + Max);
	}

	Vec3 Offset(Point P) const
	{
		Vec3 Diag = Max - Min;
		Vec3 OffsetVec = P - Min;
		return Vec3(OffsetVec.x / Diag.x, OffsetVec.y / Diag.y, OffsetVec.z / Diag.z);
	}
};

struct TriangleVertex
{
	Point Position;
	Vec3 Normal;
};

struct Triangle
{
	TriangleVertex V0;
	TriangleVertex V1;
	TriangleVertex V2;
    
    bool Intersect(Intersection& intersection, const Ray& ray)
    {
		Point P0 = V0.Position;
		Point P1 = V1.Position;
		Point P2 = V2.Position;
        Vec3 e1 = P1 - P0;
        Vec3 e2 = P2 - P0;
        
        Vec3 P = glm::cross(ray.Direction, e2);
        float det = glm::dot(e1, P);
        if (det > -EPSILON && det < EPSILON)
        {
            return false;
        }
        float inv_det = 1.f / det;
        Vec3 T = ray.Origin - P0;
        float u = glm::dot(T, P) * inv_det;
        if (u < 0.f || u > 1.f)
        {
            return false;
        }
        Vec3 Q = glm::cross(T, e1);
        float v = glm::dot(ray.Direction, Q) * inv_det;
        if (v < 0.f || u + v  > 1.f)
        {
            return false;
        }
        float t = glm::dot(e2, Q) * inv_det;
        
        if (t > 0 && t < intersection.T)
        {
            intersection.Position = ray.Origin + t * ray.Direction;
			intersection.Normal = u * V1.Normal + v * V2.Normal + (1.0f - u - v) * V0.Normal;
			intersection.T = t;
            return true;
        }
        
        return false;
    }
    
	AABB GetAABB() const 
	{
		AABB NewAABB;
		NewAABB.Min.x = std::min(std::min(V0.Position.x, V1.Position.x), V2.Position.x);
		NewAABB.Min.y = std::min(std::min(V0.Position.y, V1.Position.y), V2.Position.y);
		NewAABB.Min.z = std::min(std::min(V0.Position.z, V1.Position.z), V2.Position.z);
		NewAABB.Max.x = std::max(std::max(V0.Position.x, V1.Position.x), V2.Position.x);
		NewAABB.Max.y = std::max(std::max(V0.Position.y, V1.Position.y), V2.Position.y);
		NewAABB.Max.z = std::max(std::max(V0.Position.z, V1.Position.z), V2.Position.z);
		return NewAABB;
	}

	Point GetCentroid() const 
	{
		Point Center = V0.Position + V1.Position + V2.Position;
		return Center / 3.0f;
	}
};


