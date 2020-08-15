#pragma once
#include "./../../Source/Prefix.h"
#include "./../GraphicsAPICommon/GraphicsInterface.h"
#include "./../GraphicsAPICommon/GraphicsStructure.h"
#include "./../../Source/OutputInformationHelper.h"
#include <vulkan/vulkan.h>

#define VKFUNC(func, errorInfo) \
{ \
if(func != VK_SUCCESS)\
{\
OutputInformationHelper::OutputErrorInfomation(errorInfo);\
throw errorInfo;\
}\
}

static VkFormat GetVKTextureFormat(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::TF_R8G8B8A8UInt:
			return VkFormat::VK_FORMAT_R8G8B8A8_UINT;
		case TextureFormat::TF_R32G32B32A32SFloat:
			return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
		case TextureFormat::TF_R8G8B8A8SRGB:
			return VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::TF_B8G8R8A8SRGB:
			return VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
	}
	return VkFormat::VK_FORMAT_UNDEFINED;
}

static VkImageViewType GetVKTextureViewType(TextureDimension dimension)
{
	switch (dimension)
	{
		case TextureDimension::Texture2D:
			return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		case TextureDimension::Texture3D:
			return VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
		case TextureDimension::TextureCube:
			return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
	}
	return VkImageViewType::VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}