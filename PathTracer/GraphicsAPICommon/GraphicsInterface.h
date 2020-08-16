#pragma once

class ImageViewDesc;
class RenderingPipelineNodeDesc;
#include "./../../Source/Prefix.h"

class IDevice
{
public:
	virtual ~IDevice() {};
};

class IWindow
{
public:
	virtual ~IWindow() {};
	virtual void MainLoop() = 0;
};

class ISwapChain
{
public:
	virtual ~ISwapChain() {};
};

class IImage
{
public:
	virtual ~IImage() {};
	virtual void CreateImage(ImageDesc desc);
	virtual char* GetGPUImageHandleAddress() = 0;
};

class IImageView
{
public:
	virtual ~IImageView() {};
	virtual void CreateImageView(ImageViewDesc desc) = 0;
	virtual char* GetGPUImageViewHandleAddress() = 0;
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
	virtual void GenerateRenderingGraph(std::vector<NodePtr>& nodesVec) = 0;
};

class ITextureManager
{
public:
	virtual ~ITextureManager() {};
	virtual void CreateTexture(std::string texName);
};