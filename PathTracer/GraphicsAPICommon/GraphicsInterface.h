#pragma once

class ImageViewDesc;

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
	virtual char* GetGPUImageHandleAddress() = 0;
};

class IImageView
{
public:
	virtual ~IImageView() {};
	virtual void CreateImageView(ImageViewDesc desc) = 0;
	virtual char* GetGPUImageViewHandleAddress() = 0;
};
