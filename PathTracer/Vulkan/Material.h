#pragma once
#include "VulkanCommon.h"
#include "Shader.h"

struct VulkanMaterialShader
{
	IShader::ShaderPtr VertexShader;
	IShader::ShaderPtr FragmentShader;
	IShader::ShaderPtr GeometryShader;

	IShader::ShaderPtr ComputeShader;
};

class VulkanMaterial
{
public:
	VulkanMaterial(VulkanMaterialShader computeMaterialShader, std::string shaderGroupName);
	VulkanMaterial(VulkanMaterialShader materialShader, std::string shaderGroupName, MaterialMode materialMode);
	~VulkanMaterial();
	using MaterialPtr = std::shared_ptr<VulkanMaterial>;

private:
	void InitMaterial(VulkanMaterialShader materialShader, std::string shaderGroupName, MaterialMode materialMode);
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
	std::vector<VkDescriptorSet> GetDescriptorSet(int frameIndex, int& descCount, bool addObjectSet = false);
	Vec3 GetComputeLocalSize() { return std::static_pointer_cast<VulkanShader>(mShader.ComputeShader)->GetLocalSize(); };

public:
	std::string GetShaderGroupName() const { return mShaderGroupName; }
	void SetFloat4(std::string paramName, Vec4 value);
	void SetFloat3(std::string paramName, Vec3 value);
	void SetFloat(std::string paramName, float value);
	void SetMatrix(std::string paramName, Matrix& matrix);
	void SetImage(std::string paramName, std::string value);
	bool HasImageParam(std::string paramName);
	void BindImageAttachment(std::string paramName, std::string value);
	void Update(int frameIndex);
	void TranslateImageLayout(int frameIndex, VkCommandBuffer commandBuffer);

	void ExportOtherRateDescriptor(VkDescriptorSetLayout& setLayout, VkDescriptorPool& descPool, std::vector<VkDescriptorSet>& descSet, MaterialParams& params, std::vector<std::map<int, IBuffer::BufferPtr>>& cBuffer, int descSetIndex);
	void ExportPerObjectDescriptor(VkDescriptorSetLayout& setLayout, VkDescriptorPool& descPool, std::vector<VkDescriptorSet>& descSet, MaterialParams& params, std::vector<std::map<int, IBuffer::BufferPtr>>& cBuffer);
	void ExportPerCameraDescriptor(VkDescriptorSetLayout& setLayout, VkDescriptorPool& descPool, std::vector<VkDescriptorSet>& descSet, MaterialParams& params, std::vector<std::map<int, IBuffer::BufferPtr>>& cBuffer);
	bool IsComputeMaterial() const { return mComputeMaterial; };

private:
	std::vector<bool> mConstantBufferDirty;
	std::vector<bool> mImageDirty;

private:
	MaterialParams mParams;
	MaterialParams mPerObjectParams;
	MaterialParams mPerCameraParams;
	VulkanMaterialShader mShader;
	MaterialMode mMaterialMode;

	std::vector<VkDescriptorSetLayout> mVKDescSetLayoutVec;
	std::vector<VkPushConstantRange> mPushConstantRange;
	std::vector<VkPipelineShaderStageCreateInfo> mShaderStageInfoVec;

	VkDescriptorPoolSize mUniformBufferPoolSize;
	VkDescriptorPoolSize mImageSamplerPoolSize;
	VkDescriptorPoolSize mStorageImagePoolSize;

	VkDescriptorPoolSize mPerObjectUniformBufferPoolSize;
	VkDescriptorPoolSize mPerObjectImageSamplerPoolSize;

	VkDescriptorPoolSize mPerCameraUniformBufferPoolSize;
	VkDescriptorPoolSize mPerCameraImageSamplerPoolSize;
	VkDescriptorPoolSize mPerCameraStorageImagePoolSize;

	std::vector<VkDescriptorSetLayoutBinding> mPerObjectBindingVec;
	std::vector<int> mPerObjectBindingSize;

	std::vector<VkDescriptorSetLayoutBinding> mPerCameraBindingVec;
	std::vector<int> mPerCameraBindingSize;

	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mVKDescSetVec;
	std::vector<std::map<int, std::map<int, IBuffer::BufferPtr>>> mMaterialUniformBuffer;

	std::string mShaderGroupName;
	bool mComputeMaterial;
};