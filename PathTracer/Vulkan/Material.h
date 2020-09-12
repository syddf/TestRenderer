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
	void CreatePushConstantRange();
	void CreateShaderStageInfo();
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
	void WriteDescSet(int frameIndex);
	MaterialMode GetMaterialMode() { return mMaterialMode; }
	std::vector<VkDescriptorSetLayout>& GetSetLayouts() { return mVKDescSetLayoutVec; };
	std::vector<VkPushConstantRange>& GetPushConstantRange() { return mPushConstantRange; }
	std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStageInfo() { return mShaderStageInfoVec; }
	VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCreateInfo();
	VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo();
	VkPipelineVertexInputStateCreateInfo GetVertexInputStateCreateInfo(std::vector<VkVertexInputAttributeDescription>& attributeDescVec, VkVertexInputBindingDescription& bindingDesc);
	VkPipelineRasterizationStateCreateInfo GetRasterizationStateCreateInfo();
	VkDescriptorSet* GetDescriptorSet(int frameIndex, int& descCount);

public:
	void SetFloat4(std::string paramName, Vec4 value);
	void SetFloat3(std::string paramName, Vec3 value);
	void SetFloat(std::string paramName, float value);
	void SetMatrix(std::string paramName, Matrix& matrix);
	void SetImage(std::string paramName, std::string value);
	void BindImageAttachment(std::string paramName, std::string value);
	void Update(int frameIndex);
	void TranslateImageLayout(int frameIndex, VkCommandBuffer commandBuffer);

private:
	std::vector<bool> mConstantBufferDirty;
	std::vector<bool> mImageDirty;

private:
	MaterialParams mParams;
	VulkanMaterialShader mShader;
	MaterialMode mMaterialMode;

	std::vector<VkDescriptorSetLayout> mVKDescSetLayoutVec;
	std::vector<VkPushConstantRange> mPushConstantRange;
	std::vector<VkPipelineShaderStageCreateInfo> mShaderStageInfoVec;

	VkDescriptorPoolSize mUniformBufferPoolSize;
	VkDescriptorPoolSize mImageSamplerPoolSize;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mVKDescSetVec;
	std::vector<std::map<int, std::map<int, IBuffer::BufferPtr>>> mMaterialUniformBuffer;
};