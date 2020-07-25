#pragma once

#include "Geometry.h"

struct BVHNode
{
	AABB Bounding;
	BVHNode* LeftChild;
	BVHNode* RightChild;
	UInt32 PrimitiveOffset;
	UInt32 PrimitiveCount;
	UInt8 Axis;
};

struct LinearBVHNode
{
	AABB Bounding;
	union
	{
		int PrimitiveOffset;
		int SecondChildOffset;
	};
	UInt32 PrimitiveCount;
	UInt8 Axis;
};

struct BVHTree
{
	using BVHNodeVec = std::vector<LinearBVHNode>;
	using TriangleVec = std::vector<Triangle>;

	BVHNodeVec NodeVec;

	void BuildBVHTree(std::vector<Triangle>& TriangleVec);
	BVHNode* SplitNode(std::vector<Triangle>& TriangleVec, int Begin, int End);
	int FlattenBVHTree(BVHNode* RootNode, int& Offset);
	bool Intersect(const Ray& ray, Point& Intersection);

	int NodeCount;
	TriangleVec Triangles;
};
