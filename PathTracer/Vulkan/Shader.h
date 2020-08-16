#pragma once
#include "VulkanCommon.h"

enum VulkanShaderType
{
	VertexShader,
	FragmentShader,
	ComputeShader
};

class VulkanShader
{
public:
	VulkanShader(std::string shaderFile, VulkanShaderType shaderType);
	~VulkanShader();

public:
	void LoadShaderFromFile(const std::string& shaderFile);

private:
	VulkanShaderType mShaderType;
	VkShaderModule mShaderModule;
};