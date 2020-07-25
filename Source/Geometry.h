#pragma once
#include "Prefix.h"

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
		float DxMin = Min.x - ray.Origin.x;
		float DyMin = Min.y - ray.Origin.y;
		float DzMin = Min.z - ray.Origin.z;
		float DxMax = Max.x - ray.Origin.x;
		float DyMax = Max.y - ray.Origin.y;
		float DzMax = Max.z - ray.Origin.z;

		float TxMin = DxMin / ray.Direction.x;
		float TyMin = DyMin / ray.Direction.y;
		float TzMin = DzMin / ray.Direction.z;
		if (TxMin <= EPSILON || TyMin <= EPSILON || TzMin <= EPSILON)
			return false;
		float TxMax = DxMax / ray.Direction.x;
		float TyMax = DyMax / ray.Direction.y;
		float TzMax = DzMax / ray.Direction.z;

		return TyMax > TxMin && TyMax > TzMin && TzMax > TxMin && TzMax > TyMin && TxMax > TyMin && TxMax > TzMin;
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

	Vec3 Offset(Point P) const
	{
		Vec3 Diag = Max - Min;
		Vec3 OffsetVec = P - Min;
		return Vec3(OffsetVec.x / Diag.x, OffsetVec.y / Diag.y, OffsetVec.z / Diag.z);
	}
};

struct Triangle
{
    Point P0;
    Point P1;
    Point P2;
    
    bool Intersect(Point& intersection, const Ray& ray)
    {
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
        
        if (t > EPSILON)
        {
            intersection = ray.Origin + t * ray.Direction;
            return true;
        }
        
        return false;
    }
    
	AABB GetAABB() const 
	{
		AABB NewAABB;
		NewAABB.Min.x = std::min(std::min(P0.x, P1.x), P2.x);
		NewAABB.Min.y = std::min(std::min(P0.y, P1.y), P2.y);
		NewAABB.Min.z = std::min(std::min(P0.z, P1.z), P2.z);
		NewAABB.Max.x = std::max(std::max(P0.x, P1.x), P2.x);
		NewAABB.Max.y = std::max(std::max(P0.y, P1.y), P2.y);
		NewAABB.Max.z = std::max(std::max(P0.z, P1.z), P2.z);
		return NewAABB;
	}

	Point GetCentroid() const 
	{
		Point Center = P0 + P1 + P2;
		return Center / 3.0f;
	}
};


