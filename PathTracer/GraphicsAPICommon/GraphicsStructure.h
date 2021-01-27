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
	TF_UNDEFINED,
	TF_R8SRGB,
	TF_R8G8B8A8UInt,
	TF_R8G8B8SRGB,
	TF_R8G8B8A8SRGB,
	TF_B8G8R8A8SRGB,
	TF_R32G32B32A32SFloat,
	TF_D24US8,
	TF_R8UInt,
	TF_R32UInt
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

enum BufferUsageBits
{
	BU_VERTEX_BUFFER = 0x00000001,
	BU_INDEX_BUFFER = 0x00000002,
	BU_TRANSFER_SRC = 0x00000004,
	BU_TRANSFER_DST = 0x00000008,
	BU_UNIFORM_BUFFER = 0x00000010
};

typedef UInt32 BufferUsage;

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
	bool GenerateMipMap = false;
};

struct BufferDesc
{
	UInt32 Size;
	BufferUsage Usage;
	char* BufferData = nullptr;
};

struct TranslateBufferToImageDesc
{
	char* GPUBufferHandlePtr;
	char* GPUImageHandlePtr;
	TextureUsageBits DstTextureAspect;
	UInt32 DstTextureMipLevel;
	UInt32 DstTextureBaseLayer;
	UInt32 DstTextureLayerCount;
	UInt32 DstTextureOffset[3];
	UInt32 DstTextureExtent[3];
	UInt32 SrcBufferOffset;
};

struct TranslateBufferToBufferDesc
{
	char* GPUSrcBufferHandlePtr;
	char* GPUTarBufferHandlePtr;
	UInt32 SrcOffset;
	UInt32 DstOffset;
	UInt32 CopySize;
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
	std::vector<std::string> AttachmentName;
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

template<typename T>
struct ConstantBufferParam
{
	int Set;
	int Binding;
	int Offset;
	T Value;
};

struct ImageParam
{
	bool Attachment;
	TextureDimension ImageDimension;
	int Set;
	int Binding;
	int ArrayIndex;
	bool InnerImage;
	std::string Format;
	std::string Value;
};

struct MaterialParams
{
	std::map<std::string, ConstantBufferParam<Matrix>> MatrixParams;
	std::map<std::string, ConstantBufferParam<Vec4>> Vec4Params;
	std::map<std::string, ConstantBufferParam<Vec3>> Vec3Params;
	std::map<std::string, ConstantBufferParam<float>> FloatParams;
	std::map<std::string, ConstantBufferParam<int>> IntParams;
	std::map<std::string, ImageParam> ImageParams;
};

enum MaterialMode
{
	Normal
};