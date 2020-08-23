#pragma once
#include "VulkanCommon.h"
#include "Shader.h"

struct VulkanMaterialShader
{
	IShader::ShaderPtr VertexShader;
	IShader::ShaderPtr FragmentShader;
};

class VulkanMaterial
{
public:
	VulkanMaterial(VulkanMaterialShader materialShader);
	~VulkanMaterial();
	using MaterialPtr = std::shared_ptr<VulkanMaterial>;

private:
	void MergeShaderParams();
	void CreateVulkanDescLayout();
	void CreateVulkanDescSet();
	void CreateUniformBuffers(std::map<int, std::vector<VkDescriptorSetLayoutBinding>>& bindingMap, std::map<int, std::vector<int>>& bindingSizeMap);

	template<typename T>
	void AddConstantBufferMaterialParam(int set, int binding, int offset, std::string& name, std::map<std::string, ConstantBufferParam<T>>& bufferMap)
	{
		ConstantBufferParam<T> newParam;
		newParam.Binding = binding;
		newParam.Offset = offset;
		newParam.Set = set;
		bufferMap[name] = newParam;
	}

public:
	void WriteDescSet();

private:
	MaterialParams mParams;
	VulkanMaterialShader mShader;
	std::vector<VkDescriptorSetLayout> mVKDescSetLayoutVec;
	VkDescriptorPoolSize mUniformBufferPoolSize;
	VkDescriptorPoolSize mImageSamplerPoolSize;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mVKDescSetVec;
	std::vector<std::map<int, std::map<int, IBuffer::BufferPtr>>> mMaterialUniformBuffer;
};