#include "BVH.h"

void BVHTree::BuildBVHTree(std::vector<Triangle>& TriangleVec)
{
	NodeCount = 0;
	size_t TriangleCount = TriangleVec.size();
	BVHNode* Root = SplitNode(TriangleVec, 0, TriangleCount);
	NodeVec.resize(NodeCount);
	int Offset = 0;
	Triangles = TriangleVec;
	FlattenBVHTree(Root, Offset);
}

BVHNode * BVHTree::SplitNode(std::vector<Triangle>& TriangleVec, int Begin, int End)
{
	if (End - Begin <= 4)
	{
		BVHNode* newNode = new BVHNode;
		for (size_t Index = Begin; Index < End; Index++)
		{
			newNode->Bounding.Union(TriangleVec[Index].GetAABB());
			newNode->Axis = 0;
			newNode->LeftChild = nullptr;
			newNode->RightChild = nullptr;
			newNode->PrimitiveOffset = Begin;
			newNode->PrimitiveCount = End - Begin;
		}
		NodeCount++;
		return newNode;
	}

	AABB TotalAABB;
	for (size_t triIndex = Begin; triIndex < End; triIndex++)
	{
		TotalAABB.Union(TriangleVec[triIndex].GetAABB());
	}
	int MaxAxis = TotalAABB.GetMaxAxis();

	const int BucketCounts = 12;

	struct Bucket
	{
		AABB Bounding;
		int Count;
	};

	Bucket Buckets[BucketCounts];

	for (int i = 0; i < BucketCounts; i++)
	{
		Buckets[i].Count = 0;
		Buckets[i].Bounding.Min = Point(0, 0, 0);
		Buckets[i].Bounding.Max = Point(0, 0, 0);
	}

	for (size_t i = Begin; i < End; i++)
	{
		int BucketIndex = ((float)BucketCounts) * (TotalAABB.Offset(TriangleVec[i].GetCentroid())[MaxAxis]);
		if (BucketIndex == BucketCounts) BucketIndex--;
		Buckets[BucketIndex].Count++;
		Buckets[BucketIndex].Bounding.Union(TriangleVec[i].GetAABB());
	}

	float Cost[BucketCounts - 1];

	for (int i = 0; i < BucketCounts - 1; i++)
	{
		AABB B0, B1;
		int Count0 = 0, Count1 = 0;
		for (int j = 0; j <= i; j++)
		{
			B0.Union(Buckets[j].Bounding);
			Count0 += Buckets[j].Count;
		}
		for (int j = i + 1; j < BucketCounts; j++)
		{
			B1.Union(Buckets[j].Bounding);
			Count1 += Buckets[j].Count;
		}
		Cost[i] = 0.125f + (Count0 * B0.SurfaceArea() + Count1 * B1.SurfaceArea()) / TotalAABB.SurfaceArea();
	}

	float MinCost = Cost[0];
	int MinCostSplit = 0;

	for (int i = 1; i < BucketCounts - 1; i++)
	{
		if (Cost[i] < MinCost)
		{
			MinCost = Cost[i];
			MinCostSplit = i;
		}
	}

	int Mid = 0;

	Triangle* MidTriangle = std::partition(&TriangleVec[Begin], &TriangleVec[End - 1] + 1, 
		[&](const Triangle& t)
		{
			int BucketIndex = (float)BucketCounts * (TotalAABB.Offset(t.GetCentroid())[MaxAxis]);
			if (BucketIndex == BucketCounts)
				BucketIndex--;
			return BucketIndex <= MinCostSplit;
		}
	);
	Mid = MidTriangle - &TriangleVec[0];

	if (MinCost > End - Begin)
	{
		BVHNode* newNode = new BVHNode;
		for (size_t Index = Begin; Index < End; Index++)
		{
			newNode->Bounding.Union(TriangleVec[Index].GetAABB());
			newNode->Axis = 0;
			newNode->LeftChild = nullptr;
			newNode->RightChild = nullptr;
			newNode->PrimitiveOffset = Begin;
			newNode->PrimitiveCount = End - Begin;
		}
		NodeCount++;
		return newNode;
	}

	BVHNode* NewNode = new BVHNode;
	NodeCount++;
	NewNode->Axis = MaxAxis;
	NewNode->Bounding = TotalAABB;
	NewNode->PrimitiveCount = 0;
	NewNode->PrimitiveOffset = 0;
	NewNode->LeftChild = SplitNode(TriangleVec, Begin, Mid);
	NewNode->RightChild = SplitNode(TriangleVec, Mid, End);
	return NewNode;
}

int BVHTree::FlattenBVHTree(BVHNode * RootNode, int& Offset)
{
	LinearBVHNode& LNode = NodeVec[Offset];
	LNode.Bounding = RootNode->Bounding;
	LNode.Axis = RootNode->Axis;
	int CurOffset = Offset;
	Offset++;
	if (RootNode->PrimitiveCount > 0)
	{
		LNode.PrimitiveCount = RootNode->PrimitiveCount;
		LNode.PrimitiveOffset = RootNode->PrimitiveOffset;
	}
	else
	{
		LNode.PrimitiveCount = 0;
		FlattenBVHTree(RootNode->LeftChild, Offset);
		LNode.SecondChildOffset = FlattenBVHTree(RootNode->RightChild, Offset);
	}
	return CurOffset;
}

bool BVHTree::Intersect(const Ray & Ray, Point & Intersection)
{
	bool Hit = false;
	Vec3 InvDir(1.0f / Ray.Direction.x, 1.0f / Ray.Direction.y, 1.0f / Ray.Direction.z);
	int DirIsNeg[3] = { InvDir.x < 0, InvDir.y < 0, InvDir.z < 0 };

	int ToVisitOffset = 0, CurrentNodeIndex = 0;
	int Stack[256];
	while (true)
	{
		const LinearBVHNode& Node = NodeVec[CurrentNodeIndex];
		if (Node.Bounding.Intersect(Ray))
		{
			if (Node.PrimitiveCount > 0)
			{
				for (int i = 0; i < Node.PrimitiveCount; i++)
				{
					if (Triangles[i + Node.PrimitiveOffset].Intersect(Intersection, Ray))
						Hit = true;
				}
				if (ToVisitOffset == 0) 
					break;
				CurrentNodeIndex = Stack[--ToVisitOffset];
			}
			else 
			{
				if (DirIsNeg[Node.Axis])
				{
					Stack[ToVisitOffset++] = CurrentNodeIndex + 1;
					CurrentNodeIndex = Node.SecondChildOffset;
				}
				else 
				{
					Stack[ToVisitOffset++] = Node.SecondChildOffset;
					CurrentNodeIndex = CurrentNodeIndex + 1;
				}
			}
		}
		else 
		{
			if (ToVisitOffset == 0) break;
			CurrentNodeIndex = Stack[--ToVisitOffset];
		}
	}
	return Hit;
}
