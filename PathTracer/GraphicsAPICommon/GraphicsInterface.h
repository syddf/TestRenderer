#pragma once

#include "./../../Source/Prefix.h"

struct ImageViewDesc;
struct RenderingPipelineNodeDesc;
struct ImageDesc;
struct BufferDesc;
struct TranslateBufferToImageDesc;
class IImageView;

class IDevice
{
public:
	virtual ~IDevice() {};
};

class IWindow
{
public:
	virtual ~IWindow() {};
	virtual bool MainLoop() = 0;
};

class ISwapChain
{
public:
	virtual ~ISwapChain() {};
};

class IImage
{
public:
	using ImagePtr = std::shared_ptr<IImage>;
	virtual ~IImage() {};
	virtual void CreateImage(ImageDesc desc) = 0;
	virtual char* GetGPUImageHandleAddress() = 0;
	virtual char* GetGPUImageViewHandleAddress() = 0;
	virtual char* GetGPUImageViewHandleAddress(std::string formatName, int mipMapLevel) = 0;
	virtual char* GetSamplerHandleAddress() = 0;
	virtual void AddImageView(std::string formatName) = 0;
	virtual int GetMipMapLevelCount() = 0;
};

class IImageView
{
public:
	using ImageViewPtr = std::shared_ptr<IImageView>;
	virtual ~IImageView() {};
	virtual void CreateImageView(ImageViewDesc desc) = 0;
	virtual char* GetGPUImageViewHandleAddress() = 0;
};

class IBuffer
{
public:
	using BufferPtr = std::shared_ptr<IBuffer>;
	virtual ~IBuffer() {};
	virtual void CreateBuffer(BufferDesc desc, bool staging = false) = 0;
	virtual char* GetGPUBufferHandleAddress() = 0;
	virtual int GetBufferSize() = 0;
};

class IShader
{
public:
	using ShaderPtr = std::shared_ptr<IShader>;
	virtual ~IShader() {};
	virtual char* GetGPUShaderHandleAddress() = 0;
};

class IRenderingPipelineNode
{
public:
	virtual ~IRenderingPipelineNode() {};
	virtual void GenerateGraphicsNode(const RenderingPipelineNodeDesc& desc) = 0;
};

class IRenderingPipelineGraph
{
public:
	using NodePtr = std::shared_ptr<IRenderingPipelineNode>;

public:
	virtual ~IRenderingPipelineGraph() {};
	virtual void GenerateRenderingGraph(std::vector<RenderingPipelineNodeDesc>& nodesVec) = 0;
};

class ITranslationEngine
{
public:
	virtual ~ITranslationEngine() {};
	virtual void TranslateBufferToTexture(TranslateBufferToImageDesc desc) = 0;
};

class IMesh
{
public:
	using MeshPtr = std::shared_ptr<IMesh>;
	virtual ~IMesh() {};
	virtual char* GetVertexBufferGPUHandleAddress() = 0;
	virtual char* GetIndexBufferGPUHandleAddress() = 0;
	virtual int GetVertexCount() = 0;
	virtual int GetIndexCount() = 0;
	virtual int GetIndexBufferDataSize() = 0;
};