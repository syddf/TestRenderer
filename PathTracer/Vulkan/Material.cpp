#include "Material.h"
#include "ResourceCreator.h"
extern VkDevice gVulkanDevice;
extern UInt32 gSwapChainImageCount;

VulkanMaterial::VulkanMaterial(VulkanMaterialShader materialShader)
{
	mShader = materialShader;
	CreateVulkanDescLayout();
	CreateVulkanDescSet();
	CreatePushConstantRange();
	CreateShaderStageInfo();
	MergeShaderParams();
	WriteDescSet();

	mMaterialMode = MaterialMode::Normal;
}

VulkanMaterial::~VulkanMaterial()
{
	for (auto descLayout : mVKDescSetLayoutVec)
	{
		vkDestroyDescriptorSetLayout(gVulkanDevice, descLayout, nullptr);
	}
	vkFreeDescriptorSets(gVulkanDevice, mDescriptorPool, mVKDescSetVec.size(), mVKDescSetVec.data());
	vkDestroyDescriptorPool(gVulkanDevice, mDescriptorPool, nullptr);
}

void VulkanMaterial::MergeShaderParams()
{
	VulkanShaderParams totalParams = {};
	if (mShader.VertexShader) totalParams += std::static_pointer_cast<VulkanShader>(mShader.VertexShader)->GetShaderParams();
	if (mShader.FragmentShader) totalParams += std::static_pointer_cast<VulkanShader>(mShader.FragmentShader)->GetShaderParams();
	totalParams.Sort();
	for (auto block : totalParams.BlockVec)
	{
		if (block.StructBuffer)
		{
			for (auto param : block.ParamsVec)
			{
				if (param.Format == "float+4")
					AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, mParams.Vec4Params);
				else if (param.Format == "float+3")
					AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, mParams.Vec3Params);
				else if (param.Format == "matrix")
					AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, mParams.MatrixParams);
				else if (param.Format == "float")
					AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, mParams.FloatParams);
				else if (param.Format == "int")
					AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, mParams.IntParams);
			}
		}
		else
		{
			for (auto param : block.ParamsVec)
			{
				if (param.Format.find("Texture") != param.Format.npos)
				{
					auto GetTextureDimension = [](std::string& dimensionStr)->TextureDimension
					{
						if (dimensionStr == "2D") return TextureDimension::Texture2D;
						else if (dimensionStr == "3D") return TextureDimension::Texture3D;
						else if (dimensionStr == "Cube") return TextureDimension::TextureCube;
					};

					auto GetTextureArrayIndex = [](std::string& textureName)->int
					{
						auto numberStart = textureName.find('[');
						auto numberEnd = textureName.find(']');
						int numberCount = numberEnd - numberStart;
						std::string numberInd = textureName.substr(numberStart + 1, numberCount);
						int res = 0;
						for (int i = 0; i < numberCount; i++)
							res = res * 10 + numberInd[i] - '0';
						return res;
					};
					ImageParam imageParam;
					imageParam.Set = block.Set;
					imageParam.Binding = block.Binding;
					auto dimensionStrPos = param.Format.find("+");
					auto dimensionStr = param.Format.substr(dimensionStrPos + 1);
					imageParam.ImageDimension = GetTextureDimension(dimensionStr);
					imageParam.ArrayIndex = GetTextureArrayIndex(param.Name);
					mParams.ImageParams[param.Name] = imageParam;
				}
			}
		}
	}
}

void VulkanMaterial::CreateVulkanDescLayout()
{
	mUniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mUniformBufferPoolSize.descriptorCount = 0;
	mImageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	mImageSamplerPoolSize.descriptorCount = 0;

	std::map<int, std::vector<VkDescriptorSetLayoutBinding>> descSetLayoutMap;
	std::map<int, std::vector<int>> descBindingSizeMap;

	auto AddShader = [&](VulkanShader::VulkanShaderPtr shaderPtr, VkPipelineStageFlagBits stageFlag)->void
	{
		if (shaderPtr == nullptr)
			return;
		auto params = shaderPtr->GetShaderParams();
		auto blocks = params.BlockVec;
		for (auto block : params.BlockVec)
		{
			int binding = block.Binding;
			int set = block.Set;
			bool hasBinding = false;
			for (auto bindingDesc : descSetLayoutMap[set])
			{
				if (bindingDesc.binding == binding)
				{
					hasBinding = true;
					bindingDesc.stageFlags |= stageFlag;
					break;
				}
			}

			if (!hasBinding)
			{
				VkDescriptorSetLayoutBinding newBinding = {};
				newBinding.binding = binding;
				newBinding.stageFlags = stageFlag;
				newBinding.pImmutableSamplers = nullptr;
				if (block.StructBuffer)
				{
					newBinding.descriptorCount = 1;
					newBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					mUniformBufferPoolSize.descriptorCount++;
				}
				else
				{
					newBinding.descriptorCount = block.ParamsVec.size();
					newBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					mImageSamplerPoolSize.descriptorCount += newBinding.descriptorCount;
				}
				descSetLayoutMap[set].push_back(newBinding);
				descBindingSizeMap[set].push_back(block.BlockSize);
			}
		}
	};

	AddShader(std::static_pointer_cast<VulkanShader>(mShader.VertexShader), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
	AddShader(std::static_pointer_cast<VulkanShader>(mShader.FragmentShader), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	CreateUniformBuffers(descSetLayoutMap, descBindingSizeMap);

	for (auto bindingVec : descSetLayoutMap)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindingVec.second.size();
		layoutInfo.pBindings = bindingVec.second.data();

		VkDescriptorSetLayout descLayout;
		vkCreateDescriptorSetLayout(gVulkanDevice, &layoutInfo, nullptr, &descLayout);
		mVKDescSetLayoutVec.push_back(descLayout);
	}
}

void VulkanMaterial::CreateVulkanDescSet()
{
	mUniformBufferPoolSize.descriptorCount *= gSwapChainImageCount;
	mImageSamplerPoolSize.descriptorCount *= gSwapChainImageCount;
	VkDescriptorPoolSize descPoolSize[2] = { mUniformBufferPoolSize , mImageSamplerPoolSize };

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = mVKDescSetLayoutVec.size() * gSwapChainImageCount;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = descPoolSize;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	VKFUNC(vkCreateDescriptorPool(gVulkanDevice, &poolInfo, nullptr, &mDescriptorPool), "Create Descriptor Pool Failed.");

	mVKDescSetVec.resize(mVKDescSetLayoutVec.size() * gSwapChainImageCount);
	std::vector<VkDescriptorSetLayout> repeatedLayout;
	for (int i = 0; i < gSwapChainImageCount; i++)
	{
		repeatedLayout.insert(repeatedLayout.end(), mVKDescSetLayoutVec.begin(), mVKDescSetLayoutVec.end());
	}
	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = mDescriptorPool;
	allocateInfo.descriptorSetCount = mVKDescSetLayoutVec.size() * gSwapChainImageCount;
	allocateInfo.pSetLayouts = repeatedLayout.data();
	VKFUNC(vkAllocateDescriptorSets(gVulkanDevice, &allocateInfo, mVKDescSetVec.data()), "Allocate Descriptor Failed.");
}

void VulkanMaterial::CreatePushConstantRange()
{
	auto AddPushConstantRange = [&](VulkanShader::VulkanShaderPtr shaderPtr, VkPipelineStageFlagBits stageFlag)->void
	{
		if (shaderPtr == nullptr)
			return;
		auto shaderParams = shaderPtr->GetShaderParams();
		VkPushConstantRange range = {};
		range.stageFlags = stageFlag;
		for (auto pushConstantInfo : shaderParams.PushConstantVec)
		{
			range.offset = pushConstantInfo.offset;
			if (pushConstantInfo.format == "float+4")
				range.size = 16;
			else if (pushConstantInfo.format == "float+3")
				range.size = 12; 
			else if (pushConstantInfo.format == "matrix")
				range.size = 64;
			else if (pushConstantInfo.format == "float")
				range.size = 4;
			else if (pushConstantInfo.format == "int")
				range.size = 4;
		}
		mPushConstantRange.push_back(range);
	};

	AddPushConstantRange(std::static_pointer_cast<VulkanShader>(mShader.VertexShader), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
	AddPushConstantRange(std::static_pointer_cast<VulkanShader>(mShader.FragmentShader), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void VulkanMaterial::CreateShaderStageInfo()
{
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo.pName = "main";
	if (mShader.VertexShader)
	{
		shaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		shaderStageCreateInfo.module = *reinterpret_cast<VkShaderModule*>(mShader.VertexShader->GetGPUShaderHandleAddress());
		mShaderStageInfoVec.push_back(shaderStageCreateInfo);
	}
	else if (mShader.FragmentShader)
	{
		shaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageCreateInfo.module = *reinterpret_cast<VkShaderModule*>(mShader.FragmentShader->GetGPUShaderHandleAddress());
		mShaderStageInfoVec.push_back(shaderStageCreateInfo);
	}
}

void VulkanMaterial::CreateUniformBuffers(std::map<int, std::vector<VkDescriptorSetLayoutBinding>>& bindingMap, std::map<int, std::vector<int>>& bindingSizeMap)
{
	mMaterialUniformBuffer.resize(gSwapChainImageCount);
	for (int i = 0; i < gSwapChainImageCount; i++)
	{
		for (auto bindingVec : bindingMap)
		{
			int set = bindingVec.first;
			int vI = 0;
			for (auto binding : bindingVec.second)
			{
				int bindInd = binding.binding;
				if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
					mMaterialUniformBuffer[i][set][bindInd] = ResourceCreator::CreateUniformBuffer(bindingSizeMap[set][vI]);
				}
				vI++;
			}
		}
	}
}

void VulkanMaterial::WriteDescSet()
{
	for (int frameIndex = 0; frameIndex < gSwapChainImageCount; frameIndex++)
	{
		int startIndex = frameIndex * mVKDescSetLayoutVec.size();
		auto& uniformBufferMap = mMaterialUniformBuffer[frameIndex];
		std::vector<VkWriteDescriptorSet> descriptorSetWrite;
		for (auto set : uniformBufferMap)
		{
			int setInd = set.first;
			for (auto binding : set.second)
			{
				int bindInd = binding.first;
				auto& uniformBuffer = binding.second;
				VkDescriptorBufferInfo descBufferInfo = {};
				descBufferInfo.buffer = *reinterpret_cast<VkBuffer*>(uniformBuffer->GetGPUBufferHandleAddress());
				descBufferInfo.offset = 0;
				descBufferInfo.range = uniformBuffer->GetBufferSize();

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mVKDescSetVec[setInd + startIndex];
				descriptorWrite.dstBinding = bindInd;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &descBufferInfo;
				descriptorWrite.pImageInfo = nullptr;
				descriptorWrite.pTexelBufferView = nullptr;

				descriptorSetWrite.push_back(descriptorWrite);
			}
		}

		for (auto imageParam : mParams.ImageParams)
		{
			IImage::ImagePtr image = ResourceCreator::CreateImageFromFile(imageParam.second.Value);
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = *((VkImageView*)image->GetGPUImageViewHandleAddress());
			imageInfo.sampler = *((VkSampler*)image->GetSamplerHandleAddress());

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mVKDescSetVec[imageParam.second.Set + startIndex];
			descriptorWrite.dstBinding = imageParam.second.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = &imageInfo;
			descriptorWrite.pTexelBufferView = nullptr;
			descriptorSetWrite.push_back(descriptorWrite);
		}
		vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
	}
}

VkPipelineInputAssemblyStateCreateInfo VulkanMaterial::GetInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	return inputAssemblyStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo VulkanMaterial::GetDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
	depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateInfo.stencilTestEnable = VK_FALSE;
	return depthStencilStateInfo;
}

VkPipelineVertexInputStateCreateInfo VulkanMaterial::GetVertexInputStateCreateInfo(std::vector<VkVertexInputAttributeDescription>& attributeDescVec, VkVertexInputBindingDescription& bindingDesc)
{
	auto shader = std::static_pointer_cast<VulkanShader>(mShader.VertexShader);
	bindingDesc = shader->GetInputBindingDescription(attributeDescVec);
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescVec.data();
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescVec.size();
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDesc;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	return vertexInputStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo VulkanMaterial::GetRasterizationStateCreateInfo()
{
	VkPipelineRasterizationStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	createInfo.depthClampEnable = VK_FALSE;
	createInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.polygonMode = VK_POLYGON_MODE_FILL;
	createInfo.lineWidth = 1.0f;
	createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	createInfo.depthBiasEnable = VK_FALSE;
	return createInfo;
}

VkDescriptorSet* VulkanMaterial::GetDescriptorSet(int frameIndex, int & descCount)
{
	descCount = mVKDescSetLayoutVec.size();
	return &mVKDescSetVec[frameIndex * descCount];
}
