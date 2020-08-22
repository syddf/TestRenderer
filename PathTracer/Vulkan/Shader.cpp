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
