#pragma once

#include "./../../Source/Prefix.h"
#include "GraphicsInterface.h"

enum TextureDimension
{
	Texture2D,
	Texture3D,
	TextureCube,
	None
};

enum TextureFormat
{
	TF_R8G8B8A8UInt,
	TF_R8G8B8A8SRGB,
	TF_B8G8R8A8SRGB,
	TF_R32G32B32A32SFloat,
};

struct ImageViewDesc
{
	char* ImageHandleAddr;
	TextureDimension Dimension;
	TextureFormat Format;
	UInt32 BaseMipLevel;
	UInt32 MipLevelCount;
	UInt32 BaseArrayLayer;
	UInt32 LayerCount;
	bool IsColorTexture;
};