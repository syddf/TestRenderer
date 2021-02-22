#include "Shader.h"
#include "./../../Source/FileHelper.h"

extern VkDevice gVulkanDevice;

VulkanShader::VulkanShader(std::vector<char>& buffer, VulkanShaderParams& params, VulkanShaderType shaderType)
{
	CreateVulkanShader(buffer);
	mShaderType = shaderType;
	mVulkanShaderParams = params;
}

VulkanShader::~VulkanShader()
{
	vkDestroyShaderModule(gVulkanDevice, mShaderModule, nullptr);
}

void VulkanShader::CreateVulkanShader(std::vector<char>& buffer)
{
	VkShaderModuleCreateInfo shaderModuleInfo = {};
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());
	shaderModuleInfo.codeSize = buffer.size();

	VKFUNC(vkCreateShaderModule(gVulkanDevice, &shaderModuleInfo, nullptr, &mShaderModule), "Create Shader Failed.");
}

VkVertexInputBindingDescription VulkanShader::GetInputBindingDescription(std::vector<VkVertexInputAttributeDescription>& attributeDescVec)
{
	VkVertexInputBindingDescription inputBindingDescription = {};
	std::sort(mVulkanShaderParams.InputVec.begin(), mVulkanShaderParams.InputVec.end());
	attributeDescVec.resize(mVulkanShaderParams.InputVec.size());
	auto GetInputParamSize = [&](const ShaderInputInfo& inputInfo)->int
	{
		if (inputInfo.format == "float+4")
			return 4 * sizeof(float);
		else if (inputInfo.format == "float+3")
			return 3 * sizeof(float);
		else if (inputInfo.format == "float+2")
			return 2 * sizeof(float);
	};
	auto GetInputParamFormat = [&](const ShaderInputInfo& inputInfo)->VkFormat
	{
		if (inputInfo.format == "float+4")
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		if (inputInfo.format == "float+3")
			return VK_FORMAT_R32G32B32_SFLOAT;
		if (inputInfo.format == "float+2")
			return VK_FORMAT_R32G32_SFLOAT;
	};

	static std::map<std::string, int> sOffsetMap = 
	{
		{ "inPosition", 0 },
		{ "inNormal", 12 },
		{ "inTangent", 24 },
		{ "inBiTangent", 36 },
		{ "inTexCoord", 48 },
	};

	int totalSize = 56;

	for (int i = 0; i < mVulkanShaderParams.InputVec.size(); i++)
	{
		auto& shaderInputInfo = mVulkanShaderParams.InputVec[i];
		attributeDescVec[i].binding = 0;
		attributeDescVec[i].location = i;
		attributeDescVec[i].offset = sOffsetMap[shaderInputInfo.name];
		attributeDescVec[i].format = GetInputParamFormat(shaderInputInfo);
	}
	inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	inputBindingDescription.stride = totalSize;
	inputBindingDescription.binding = 0; // may need to be modified
	return inputBindingDescription;
}
