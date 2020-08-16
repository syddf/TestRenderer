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

static VkImageType GetVKTextureType(TextureDimension dimension)
{
	switch (dimension)
	{
		case TextureDimension::Texture2D:
			return VkImageType::VK_IMAGE_TYPE_2D;
		case TextureDimension::Texture3D:
			return VkImageType::VK_IMAGE_TYPE_3D;
		case TextureDimension::TextureCube:
			return VkImageType::VK_IMAGE_TYPE_2D;
	}
	return VkImageType::VK_IMAGE_TYPE_MAX_ENUM;
}

static VkAttachmentLoadOp GetVKLoadOp(AttachmentOperator attachmentOp)
{
	switch (attachmentOp)
	{
		case AttachmentOperator::AO_LOAD:
			return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		case AttachmentOperator::AO_CLEAR:
			return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		case AttachmentOperator::AO_DONT_CARE:
			return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

static VkAttachmentStoreOp GetVKStoreOp(AttachmentOperator attachmentOp)
{
	switch (attachmentOp)
	{
		case AttachmentOperator::AO_STORE:
			return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		case AttachmentOperator::AO_DONT_CARE:
			return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

static VkPipelineBindPoint GetVKBindPoint(PipelineBindPoint bindPoint)
{
	switch (bindPoint)
	{
		case PipelineBindPoint::BP_GRAPHICS:
			return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PipelineBindPoint::BP_COMPUTE:
			return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
	}
	return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_MAX_ENUM;
}

static VkImageUsageFlagBits GetVKImageUsageFlagBits(TextureUsageBits usage)
{
	switch (usage)
	{
		case TextureUsageBits::TU_COLOR_ATTACHMENT:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case TextureUsageBits::TU_DEPTH_STENCIL:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case TextureUsageBits::TU_SHADER_RESOURCE:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case TextureUsageBits::TU_TRANSFER_SRC:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case TextureUsageBits::TU_TRENSFER_DST:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	return VkImageUsageFlagBits::VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
}

static VkImageUsageFlags GetVKImageUsageFlags(TextureUsage usage)
{
	UInt32 res = 0;
	if (usage & TU_COLOR_ATTACHMENT)
	{
		res |= GetVKImageUsageFlagBits(TU_COLOR_ATTACHMENT);
	}
	if (usage & TU_DEPTH_STENCIL)
	{
		res |= GetVKImageUsageFlagBits(TU_DEPTH_STENCIL);
	}
	if (usage & TU_SHADER_RESOURCE)
	{
		res |= GetVKImageUsageFlagBits(TU_SHADER_RESOURCE);
	}
	if (usage & TU_TRANSFER_SRC)
	{
		res |= GetVKImageUsageFlagBits(TU_TRANSFER_SRC);
	}
	if (usage & TU_TRANSFER_DST)
	{
		res |= GetVKImageUsageFlagBits(TU_TRANSFER_DST);
	}
	return res;
}