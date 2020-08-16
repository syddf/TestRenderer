#include "Shader.h"
#include "./../../Source/FileHelper.h"

extern VkDevice gVulkanDevice;

VulkanShader::VulkanShader(std::string shaderFile, VulkanShaderType shaderType)
{
	LoadShaderFromFile(shaderFile);
	mShaderType = shaderType;
}

VulkanShader::~VulkanShader()
{
	vkDestroyShaderModule(gVulkanDevice, mShaderModule, nullptr);
}

void VulkanShader::LoadShaderFromFile(const std::string & shaderFile)
{
	FileHelper helper(shaderFile, FileHelper::FileMode::FileRead);
	std::vector<char> buffer;
	helper.ReadAllBinaryFile(buffer);

	VkShaderModuleCreateInfo shaderModuleInfo = {};
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());
	shaderModuleInfo.codeSize = buffer.size();

	VKFUNC(vkCreateShaderModule(gVulkanDevice, &shaderModuleInfo, nullptr, &mShaderModule), "Create Shader Failed.");
}
