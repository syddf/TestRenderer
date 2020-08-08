#pragma once

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