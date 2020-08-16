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

enum AttachmentOperator
{
	AO_CLEAR,
	AO_LOAD,
	AO_STORE,
	AO_DONT_CARE
};

enum TextureLayout
{
	TL_UNDEFINED,
	TL_PRESENT_SRC,
	TL_COLOR_ATTACHMENT
};

enum TextureUsageBits
{
	TU_COLOR_ATTACHMENT = 0x00000001,
	TU_DEPTH_STENCIL = 0x00000002,
	TU_SHADER_RESOURCE = 0x00000004,
	TU_TRANSFER_SRC = 0x00000008,
	TU_TRANSFER_DST = 0x00000010
};

typedef UInt32 TextureUsage;

enum PipelineBindPoint
{
	BP_GRAPHICS,
	BP_COMPUTE
};

struct ImageDesc
{
	char* ImageData = nullptr;
	UInt32 Width;
	UInt32 Height;
	UInt32 Depth;
	UInt32 MipLevels;
	UInt32 ArrayLayers;
	TextureDimension Dimension;
	TextureFormat Format;
	TextureUsage Usage;
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

struct FrameBufferDesc
{
	std::vector<char*> AttachmentImageViewHandlePtr;
	UInt32 Width;
	UInt32 Height;
};

struct AttachmentDesc
{
	TextureFormat Format;
	AttachmentOperator LoadOp;
	AttachmentOperator StoreOp;
	TextureUsage Usage;
};

struct FrameBufferLayoutDesc
{
	std::vector<AttachmentDesc> AttachmentDesc;
};

struct RenderingPipelineNodeDesc
{
	FrameBufferLayoutDesc FrameBufferLayoutDesc;
	FrameBufferDesc FrameBufferDesc;
	std::vector<char*> DependingAttachmentViewPtr;
	std::vector<int> DependingNodeIndex;
	PipelineBindPoint BindPoint;
	bool AttachToWindowNode;
	bool AffectOtherNode;
};