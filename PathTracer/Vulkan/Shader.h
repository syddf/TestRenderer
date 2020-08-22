#pragma once
#include "VulkanCommon.h"
#include "./../../AssetImporter/ImportSPIRVShaderData.h"

enum VulkanShaderType
{
	VertexShader,
	FragmentShader,
	ComputeShader
};

struct VulkanShaderParams
{
	std::vector<ShaderBlockInfo> BlockVec;
	std::vector<ShaderInputInfo> InputVec;
	std::vector<ShaderPushConstantInfo> PushConstantVec;

	VulkanShaderParams& operator+=(const VulkanShaderParams& params)
	{
		BlockVec.insert(BlockVec.end(), params.BlockVec.begin(), params.BlockVec.end());
		InputVec.insert(InputVec.end(), params.InputVec.begin(), params.InputVec.end());
		PushConstantVec.insert(PushConstantVec.end(), params.PushConstantVec.begin(), params.PushConstantVec.end());
		return *this;
	}

	void Sort()
	{
		std::sort(BlockVec.begin(), BlockVec.end());
		std::sort(InputVec.begin(), InputVec.end());
		std::sort(PushConstantVec.begin(), PushConstantVec.end());
	}
};

class VulkanShader : public IShader
{
public:
	VulkanShader(std::vector<char>& buffer, VulkanShaderParams& params, VulkanShaderType shaderType);
	~VulkanShader();
	using VulkanShaderPtr = std::shared_ptr<VulkanShader>;

public:
	void CreateVulkanShader(std::vector<char>& buffer);
	VulkanShaderParams GetShaderParams() { return mVulkanShaderParams;  };
	char* GetGPUShaderHandleAddress() { return reinterpret_cast<char*>(&mShaderModule); };

private:
	VulkanShaderType mShaderType;
	VkShaderModule mShaderModule;
	VulkanShaderParams mVulkanShaderParams;
};