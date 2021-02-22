#include "Material.h"
#include "ResourceCreator.h"
extern VkDevice gVulkanDevice;
extern UInt32 gSwapChainImageCount;

const int PerObjDescSetIndex = 1;
const int PerCameraDescSetIndex = 2;

template<typename T>
void UpdateConstantBufferParam(int frameIndex, int offset, IBuffer::BufferPtr buffer, T& value)
{
	char* data;
	auto memory = std::dynamic_pointer_cast<VulkanBuffer>(buffer)->GetGPUBufferMemory();
	vkMapMemory(gVulkanDevice, memory, offset, sizeof(T), 0, &((void*)data));
	memcpy(data, &value, sizeof(T));
	vkUnmapMemory(gVulkanDevice, memory);
}


VulkanMaterial::VulkanMaterial(VulkanMaterialShader computeMaterialShader, std::string shaderGroupName)
{
	assert(computeMaterialShader.VertexShader == nullptr);
	assert(computeMaterialShader.FragmentShader == nullptr);
	assert(computeMaterialShader.GeometryShader == nullptr);
	assert(computeMaterialShader.ComputeShader != nullptr);
	InitMaterial(computeMaterialShader, shaderGroupName, MaterialMode::Normal);
	mComputeMaterial = true;
}

VulkanMaterial::VulkanMaterial(VulkanMaterialShader materialShader, std::string shaderGroupName, MaterialMode materialMode)
{
	assert(materialShader.ComputeShader == nullptr);
	InitMaterial(materialShader, shaderGroupName, materialMode);
	mComputeMaterial = false;
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

void VulkanMaterial::InitMaterial(VulkanMaterialShader materialShader, std::string shaderGroupName, MaterialMode materialMode)
{
	mShader = materialShader;
	mShaderGroupName = shaderGroupName;
	mMaterialMode = materialMode;

	CreateVulkanDescLayout();
	CreateVulkanDescSet();
	CreatePushConstantRange();
	CreateShaderStageInfo();
	MergeShaderParams();
	WriteDescSet();

	mConstantBufferDirty.resize(gSwapChainImageCount, false);
	mImageDirty.resize(gSwapChainImageCount, false);
	mComputeMaterial = false;
}

void VulkanMaterial::MergeShaderParams()
{
	VulkanShaderParams totalParams = {};
	if (mShader.VertexShader) totalParams += std::static_pointer_cast<VulkanShader>(mShader.VertexShader)->GetShaderParams();
	if (mShader.FragmentShader) totalParams += std::static_pointer_cast<VulkanShader>(mShader.FragmentShader)->GetShaderParams();
	if (mShader.GeometryShader) totalParams += std::static_pointer_cast<VulkanShader>(mShader.GeometryShader)->GetShaderParams();
	if (mShader.ComputeShader) totalParams += std::static_pointer_cast<VulkanShader>(mShader.ComputeShader)->GetShaderParams();
	totalParams.Sort();

	auto AddParams = [&](ShaderBlockInfo & block, ShaderParameter& param, MaterialParams& allParams)->void
	{
		if (param.Format == "float+4")
			AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, allParams.Vec4Params);
		else if (param.Format == "float+3")
			AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, allParams.Vec3Params);
		else if (param.Format == "matrix")
			AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, allParams.MatrixParams);
		else if (param.Format == "float")
			AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, allParams.FloatParams);
		else if (param.Format == "int")
			AddConstantBufferMaterialParam(block.Set, block.Binding, param.Offset, param.Name, allParams.IntParams);
	};

	for (auto block : totalParams.BlockVec)
	{
		if (block.StructBuffer)
		{
			for (auto param : block.ParamsVec)
			{
				AddParams(block, param, mParams);

				if (block.Set == PerObjDescSetIndex)
					AddParams(block, param, mPerObjectParams);

				if (block.Set == PerCameraDescSetIndex)
					AddParams(block, param, mPerCameraParams);
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
						int numberCount = numberEnd - numberStart - 1;
						if (numberCount <= 0)
							return 0;
						std::string numberInd = textureName.substr(numberStart + 1, numberCount);
						int res = 0;
						for (int i = 0; i < numberCount; i++)
							res = res * 10 + numberInd[i] - '0';
						return res;
					};
					ImageParam imageParam;
					imageParam.Set = block.Set;
					imageParam.Binding = block.Binding;
					auto dimensionStrPos = param.Format.find_first_of("+");
					auto dimensionStrEndPos = param.Format.find_last_of("+");
					auto dimensionStr = param.Format.substr(dimensionStrPos + 1, dimensionStrEndPos - dimensionStrPos - 1);
					imageParam.ImageDimension = GetTextureDimension(dimensionStr);
					imageParam.ArrayIndex = GetTextureArrayIndex(param.Name);
					imageParam.Attachment = false;
					imageParam.Format = param.Format.substr(dimensionStrEndPos + 1);
					imageParam.InnerImage = false;					
					imageParam.CombinedSampler = true;
					auto nameLength = param.Name.length();
					if (param.Name.find("_IN") != param.Name.npos)
					{
						imageParam.InnerImage = true;
					}
					if (imageParam.Format != "Unknown")
					{
						imageParam.CombinedSampler = false;
					}
					mParams.ImageParams[param.Name] = imageParam;

					if (block.Set == PerObjDescSetIndex)
						mPerObjectParams.ImageParams[param.Name] = imageParam;

					if (block.Set == PerCameraDescSetIndex)
						mPerCameraParams.ImageParams[param.Name] = imageParam;
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
	mStorageImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	mStorageImagePoolSize.descriptorCount = 0;

	mPerObjectUniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mPerObjectUniformBufferPoolSize.descriptorCount = 0;
	mPerObjectImageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	mPerObjectImageSamplerPoolSize.descriptorCount = 0;

	mPerCameraUniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mPerCameraUniformBufferPoolSize.descriptorCount = 0;
	mPerCameraImageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	mPerCameraImageSamplerPoolSize.descriptorCount = 0;
	mPerCameraStorageImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	mPerCameraStorageImagePoolSize.descriptorCount = 0;

	std::map<int, std::vector<VkDescriptorSetLayoutBinding>> descSetLayoutMap;
	std::map<int, std::vector<int>> descBindingSizeMap;

	auto AddShader = [&](VulkanShader::VulkanShaderPtr shaderPtr, VkShaderStageFlagBits stageFlag)->void
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
			for (auto& bindingDesc : descSetLayoutMap[set])
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
					if (set == PerObjDescSetIndex)
						mPerObjectUniformBufferPoolSize.descriptorCount++;
					if (set == PerCameraDescSetIndex)
						mPerCameraUniformBufferPoolSize.descriptorCount++;
				}
				else
				{
					if (block.ParamsVec[0].Combined)
					{
						newBinding.descriptorCount = block.ParamsVec.size();
						newBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						mImageSamplerPoolSize.descriptorCount += block.ParamsVec.size();
						if (set == PerObjDescSetIndex)
							mPerObjectImageSamplerPoolSize.descriptorCount += block.ParamsVec.size();
						if (set == PerCameraDescSetIndex)
							mPerCameraImageSamplerPoolSize.descriptorCount += block.ParamsVec.size();
					}
					else
					{
						newBinding.descriptorCount = block.ParamsVec.size();
						newBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						if (set == PerCameraDescSetIndex)
							mPerCameraStorageImagePoolSize.descriptorCount += block.ParamsVec.size();
						mStorageImagePoolSize.descriptorCount += block.ParamsVec.size();
					}
				}

				descSetLayoutMap[set].push_back(newBinding);
				descBindingSizeMap[set].push_back(block.BlockSize);

				if (set == PerObjDescSetIndex)
				{
					mPerObjectBindingVec.push_back(newBinding);
					mPerObjectBindingSize.push_back(block.BlockSize);
				}
				if (set == PerCameraDescSetIndex)
				{
					mPerCameraBindingVec.push_back(newBinding);
					mPerCameraBindingSize.push_back(block.BlockSize);
				}
			}
		}
	};

	AddShader(std::static_pointer_cast<VulkanShader>(mShader.VertexShader), VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	AddShader(std::static_pointer_cast<VulkanShader>(mShader.FragmentShader), VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
	AddShader(std::static_pointer_cast<VulkanShader>(mShader.GeometryShader), VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT);
	AddShader(std::static_pointer_cast<VulkanShader>(mShader.ComputeShader), VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);

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
	mStorageImagePoolSize.descriptorCount *= gSwapChainImageCount;
	std::vector<VkDescriptorPoolSize> descPoolSize;
	if (mUniformBufferPoolSize.descriptorCount != 0)
		descPoolSize.push_back(mUniformBufferPoolSize);
	if (mImageSamplerPoolSize.descriptorCount != 0)
		descPoolSize.push_back(mImageSamplerPoolSize);
	if (mStorageImagePoolSize.descriptorCount != 0)
		descPoolSize.push_back(mStorageImagePoolSize);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = mVKDescSetLayoutVec.size() * gSwapChainImageCount;
	poolInfo.poolSizeCount = descPoolSize.size();
	poolInfo.pPoolSizes = descPoolSize.data();
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
	auto AddPushConstantRange = [&](VulkanShader::VulkanShaderPtr shaderPtr, VkShaderStageFlagBits stageFlag)->void
	{
		if (shaderPtr == nullptr)
			return;
		auto shaderParams = shaderPtr->GetShaderParams();
		VkPushConstantRange range = {};
		range.stageFlags = stageFlag;
		if (shaderParams.PushConstantVec.empty()) return;
		int offset = MAXF;
		int size = -MAXF;
		for (auto pushConstantInfo : shaderParams.PushConstantVec)
		{
			offset = std::min(offset, pushConstantInfo.offset);
			if (pushConstantInfo.format == "float+4")
				size = std::max(size, pushConstantInfo.offset + 16);
			else if (pushConstantInfo.format == "float+3")
				size = std::max(size, pushConstantInfo.offset + 12);
			else if (pushConstantInfo.format == "matrix")
				size = std::max(size, pushConstantInfo.offset + 64);
			else if (pushConstantInfo.format == "float")
				size = std::max(size, pushConstantInfo.offset + 4);
			else if (pushConstantInfo.format == "int")
				size = std::max(size, pushConstantInfo.offset + 4);
		}
		range.size = size - offset;
		mPushConstantRange.push_back(range);
	};

	AddPushConstantRange(std::static_pointer_cast<VulkanShader>(mShader.VertexShader), VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	AddPushConstantRange(std::static_pointer_cast<VulkanShader>(mShader.GeometryShader), VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT);
	AddPushConstantRange(std::static_pointer_cast<VulkanShader>(mShader.FragmentShader), VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
	AddPushConstantRange(std::static_pointer_cast<VulkanShader>(mShader.ComputeShader), VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);

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
	if (mShader.FragmentShader)
	{
		shaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageCreateInfo.module = *reinterpret_cast<VkShaderModule*>(mShader.FragmentShader->GetGPUShaderHandleAddress());
		mShaderStageInfoVec.push_back(shaderStageCreateInfo);
	}
	if (mShader.GeometryShader)
	{
		shaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		shaderStageCreateInfo.module = *reinterpret_cast<VkShaderModule*>(mShader.GeometryShader->GetGPUShaderHandleAddress());
		mShaderStageInfoVec.push_back(shaderStageCreateInfo);
	}
	if (mShader.ComputeShader)
	{
		shaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageCreateInfo.module = *reinterpret_cast<VkShaderModule*>(mShader.ComputeShader->GetGPUShaderHandleAddress());
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
		WriteDescSet(frameIndex);
	}
}

void VulkanMaterial::WriteDescSet(int frameIndex)
{
	int startIndex = frameIndex * mVKDescSetLayoutVec.size();
	auto& uniformBufferMap = mMaterialUniformBuffer[frameIndex];
	std::vector<VkWriteDescriptorSet> descriptorSetWrite;
	std::vector<VkDescriptorBufferInfo> descBufferInfoVec;
	std::vector<VkDescriptorImageInfo> descImageInfoVec;
	descBufferInfoVec.reserve(256);
	descImageInfoVec.reserve(256);
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
			descBufferInfoVec.push_back(descBufferInfo);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mVKDescSetVec[setInd + startIndex];
			descriptorWrite.dstBinding = bindInd;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &(descBufferInfoVec.back());
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			descriptorSetWrite.push_back(descriptorWrite);
		}
	}

	for (auto imageParam : mParams.ImageParams)
	{
		if (imageParam.second.Set == PerCameraDescSetIndex)
		{
			continue;
		}
		IImage::ImagePtr image;
		if (imageParam.second.Attachment)
		{
			auto attach = ResourceCreator::GetAttachment(imageParam.second.Value);
			image = attach->GetImage(frameIndex);
		}
		else
		{
			image = ResourceCreator::CreateImageFromFile(imageParam.second.Value);
		}
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = *((VkImageView*)image->GetGPUImageViewHandleAddress());
		imageInfo.sampler = *((VkSampler*)image->GetSamplerHandleAddress());
		descImageInfoVec.push_back(imageInfo);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mVKDescSetVec[imageParam.second.Set + startIndex];
		descriptorWrite.dstBinding = imageParam.second.Binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &descImageInfoVec.back();
		descriptorWrite.pTexelBufferView = nullptr;
		descriptorSetWrite.push_back(descriptorWrite);
	}
	vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
}

VkPipelineInputAssemblyStateCreateInfo VulkanMaterial::GetInputAssemblyStateCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	if (mShader.GeometryShader)
	{
		auto primitive = std::static_pointer_cast<VulkanShader>(mShader.GeometryShader)->GetPrimitiveInput();
		if (primitive == GeometryPrimitiveInput::PI_POINTS)
		{
			inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}
	}
	return inputAssemblyStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo VulkanMaterial::GetDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
	depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateInfo.stencilTestEnable = VK_FALSE;
	if (mMaterialMode == MaterialMode::NoAttachment)
	{
		depthStencilStateInfo.depthWriteEnable = VK_FALSE;
		depthStencilStateInfo.depthTestEnable = VK_FALSE;
	}
	else
	{
		depthStencilStateInfo.depthWriteEnable = VK_TRUE;
		depthStencilStateInfo.depthTestEnable = VK_TRUE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	}
	return depthStencilStateInfo;
}

VkPipelineVertexInputStateCreateInfo VulkanMaterial::GetVertexInputStateCreateInfo(std::vector<VkVertexInputAttributeDescription>& attributeDescVec, VkVertexInputBindingDescription& bindingDesc)
{
	auto shader = std::static_pointer_cast<VulkanShader>(mShader.VertexShader);
	bindingDesc = shader->GetInputBindingDescription(attributeDescVec);
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (bindingDesc.stride != 0)
	{
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescVec.data();
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescVec.size();
		vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	}
	else
	{
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
	}
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
	createInfo.cullMode = VK_CULL_MODE_NONE;
	createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	createInfo.depthBiasEnable = VK_FALSE;
	return createInfo;
}

std::vector<VkDescriptorSet> VulkanMaterial::GetDescriptorSet(int frameIndex, int & descCount, bool addObjectSet)
{
	std::vector<VkDescriptorSet> descSetVec;
	descCount = mVKDescSetLayoutVec.size();
	for (int i = 0; i < descCount; i++)
	{
		if ((i == PerObjDescSetIndex && !addObjectSet) || i == PerCameraDescSetIndex)
			continue;
		descSetVec.push_back(mVKDescSetVec[frameIndex * descCount + i]);
	}
	return descSetVec;
}

void VulkanMaterial::SetFloat4(std::string paramName, Vec4 value)
{
	//assert(mParams.Vec4Params.find(paramName) != mParams.Vec4Params.end());
	mParams.Vec4Params[paramName].Value = value;
	for (int i = 0; i < mConstantBufferDirty.size(); i++)
		mConstantBufferDirty[i] = true;
}

void VulkanMaterial::SetFloat3(std::string paramName, Vec3 value)
{
	//assert(mParams.Vec3Params.find(paramName) != mParams.Vec3Params.end());
	mParams.Vec3Params[paramName].Value = value;
	for (int i = 0; i < mConstantBufferDirty.size(); i++)
		mConstantBufferDirty[i] = true;
}

void VulkanMaterial::SetFloat(std::string paramName, float value)
{
	//assert(mParams.FloatParams.find(paramName) != mParams.FloatParams.end());
	mParams.FloatParams[paramName].Value = value;
	for (int i = 0; i < mConstantBufferDirty.size(); i++)
		mConstantBufferDirty[i] = true;
}

void VulkanMaterial::SetMatrix(std::string paramName, Matrix & matrix)
{
	//assert(mParams.MatrixParams.find(paramName) != mParams.MatrixParams.end());
	mParams.MatrixParams[paramName].Value = matrix;
	for (int i = 0; i < mConstantBufferDirty.size(); i++)
		mConstantBufferDirty[i] = true;
}

void VulkanMaterial::SetImage(std::string paramName, std::string value)
{
	assert(mParams.ImageParams.find(paramName) != mParams.ImageParams.end());
	mParams.ImageParams[paramName].Value = value;
	mParams.ImageParams[paramName].Attachment = false;
	for (int i = 0; i < mImageDirty.size(); i++)
		mImageDirty[i] = true;
}

bool VulkanMaterial::HasImageParam(std::string paramName)
{
	return mParams.ImageParams.find(paramName) != mParams.ImageParams.end();
}

void VulkanMaterial::BindImageAttachment(std::string paramName, std::string value)
{
	assert(mParams.ImageParams.find(paramName) != mParams.ImageParams.end());
	mParams.ImageParams[paramName].Value = value;
	mParams.ImageParams[paramName].Attachment = true;
	mImageDirty.resize(gSwapChainImageCount, true);
}

void VulkanMaterial::Update(int frameIndex)
{
	if (mConstantBufferDirty[frameIndex])
	{
		mConstantBufferDirty[frameIndex] = false;
		for (auto param : mParams.Vec4Params)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialUniformBuffer[frameIndex][param.second.Set][param.second.Binding], param.second.Value);
		for (auto param : mParams.Vec3Params)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialUniformBuffer[frameIndex][param.second.Set][param.second.Binding], param.second.Value);
		for (auto param : mParams.FloatParams)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialUniformBuffer[frameIndex][param.second.Set][param.second.Binding], param.second.Value);
		for (auto param : mParams.MatrixParams)
			UpdateConstantBufferParam(frameIndex, param.second.Offset, mMaterialUniformBuffer[frameIndex][param.second.Set][param.second.Binding], param.second.Value);
	}
	if (mImageDirty[frameIndex])
	{
		mImageDirty[frameIndex] = false;
		WriteDescSet(frameIndex);
	}
}

void VulkanMaterial::TranslateImageLayout(int frameIndex, VkCommandBuffer commandBuffer)
{
	for (auto param : mParams.ImageParams)
	{
		if (param.second.Attachment)
		{
			auto attach = ResourceCreator::GetAttachment(param.second.Value);
			auto image = attach->GetImage(frameIndex);
			image->TranslateImageLayout(commandBuffer, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 0);
		}
		else
		{
			IImage::ImagePtr image = ResourceCreator::CreateImageFromFile(param.second.Value);
			std::dynamic_pointer_cast<VulkanImage>(image)->TranslateImageLayout(commandBuffer, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 0);
		}
	}
}

void VulkanMaterial::ExportOtherRateDescriptor(VkDescriptorSetLayout & setLayout, VkDescriptorPool & descPool, std::vector<VkDescriptorSet>& descSet, MaterialParams & params, std::vector<std::map<int, IBuffer::BufferPtr>>& cBuffer, int descSetIndex)
{
	setLayout = mVKDescSetLayoutVec[descSetIndex];

	VkDescriptorPoolSize exportDescPoolBufferSize = descSetIndex == PerObjDescSetIndex ? mPerObjectUniformBufferPoolSize : mPerCameraUniformBufferPoolSize;
	exportDescPoolBufferSize.descriptorCount *= gSwapChainImageCount;
	VkDescriptorPoolSize exportDescPoolImageSize = descSetIndex == PerObjDescSetIndex ? mPerObjectImageSamplerPoolSize : mPerCameraImageSamplerPoolSize;
	exportDescPoolImageSize.descriptorCount *= gSwapChainImageCount;
	VkDescriptorPoolSize exportDescPoolStorageImageSize = {};
	if(descSetIndex == PerCameraDescSetIndex)
		exportDescPoolStorageImageSize = mPerCameraStorageImagePoolSize;
	exportDescPoolStorageImageSize.descriptorCount *= gSwapChainImageCount;

	std::vector<VkDescriptorPoolSize> descPoolSize;
	if (exportDescPoolBufferSize.descriptorCount > 0)
		descPoolSize.push_back(exportDescPoolBufferSize);
	if (exportDescPoolImageSize.descriptorCount > 0)
		descPoolSize.push_back(exportDescPoolImageSize);
	if (exportDescPoolStorageImageSize.descriptorCount > 0)
		descPoolSize.push_back(exportDescPoolStorageImageSize);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = gSwapChainImageCount;
	poolInfo.poolSizeCount = descPoolSize.size();
	poolInfo.pPoolSizes = descPoolSize.data();
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	VKFUNC(vkCreateDescriptorPool(gVulkanDevice, &poolInfo, nullptr, &descPool), "Create Descriptor Pool Failed.");

	descSet.resize(gSwapChainImageCount);
	std::vector<VkDescriptorSetLayout> repeatedLayout;
	for (int i = 0; i < gSwapChainImageCount; i++)
		repeatedLayout.push_back(setLayout);
	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = descPool;
	allocateInfo.descriptorSetCount = gSwapChainImageCount;
	allocateInfo.pSetLayouts = repeatedLayout.data();
	VKFUNC(vkAllocateDescriptorSets(gVulkanDevice, &allocateInfo, descSet.data()), "Allocate Descriptor Failed.");

	params = descSetIndex == 1 ? mPerObjectParams : mPerCameraParams;

	cBuffer.resize(gSwapChainImageCount);
	for (int i = 0; i < gSwapChainImageCount; i++)
	{
		int vI = 0;
		auto& bindingVec = descSetIndex == 1 ? mPerObjectBindingVec : mPerCameraBindingVec;
		auto& bindingSizeVec = descSetIndex == 1 ? mPerObjectBindingSize : mPerCameraBindingSize;
		for (auto binding : bindingVec)
		{
			int bindInd = binding.binding;
			if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				cBuffer[i][bindInd] = ResourceCreator::CreateUniformBuffer(bindingSizeVec[vI]);
			}
			vI++;
		}
	}

	std::vector<VkWriteDescriptorSet> descriptorSetWrite;
	std::vector<VkDescriptorBufferInfo> bufferInfoVec;
	bufferInfoVec.reserve(256);
	for (int i = 0; i < gSwapChainImageCount; i++)
	{
		for (auto binding : cBuffer[i])
		{
			int bindInd = binding.first;
			auto& uniformBuffer = binding.second;
			VkDescriptorBufferInfo descBufferInfo = {};
			descBufferInfo.buffer = *reinterpret_cast<VkBuffer*>(uniformBuffer->GetGPUBufferHandleAddress());
			descBufferInfo.offset = 0;
			descBufferInfo.range = uniformBuffer->GetBufferSize();
			bufferInfoVec.push_back(descBufferInfo);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descSet[i];
			descriptorWrite.dstBinding = bindInd;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfoVec.back();
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			descriptorSetWrite.push_back(descriptorWrite);
		}
	}
	vkUpdateDescriptorSets(gVulkanDevice, descriptorSetWrite.size(), descriptorSetWrite.data(), 0, nullptr);
}

void VulkanMaterial::ExportPerObjectDescriptor(VkDescriptorSetLayout & setLayout, VkDescriptorPool & descPool, std::vector<VkDescriptorSet>& descSet, MaterialParams & params, std::vector<std::map<int, IBuffer::BufferPtr>>& cBuffer)
{
	ExportOtherRateDescriptor(setLayout, descPool, descSet, params, cBuffer, PerObjDescSetIndex);
}

void VulkanMaterial::ExportPerCameraDescriptor(VkDescriptorSetLayout & setLayout, VkDescriptorPool & descPool, std::vector<VkDescriptorSet>& descSet, MaterialParams & params, std::vector<std::map<int, IBuffer::BufferPtr>>& cBuffer)
{
	ExportOtherRateDescriptor(setLayout, descPool, descSet, params, cBuffer, PerCameraDescSetIndex);
}
