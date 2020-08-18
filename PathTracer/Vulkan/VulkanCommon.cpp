#include "VulkanCommon.h"

extern VkPhysicalDevice gVulkanPhysicalDevice;

UInt32 GetVKMemoryType(UInt32 typeFilter, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties physicalMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(gVulkanPhysicalDevice, &physicalMemoryProperties);
	for (auto i = 0; i < physicalMemoryProperties.memoryTypeCount; i ++)
	{
		if ((1 << i) & typeFilter)
		{
			if ((physicalMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
			{
				return i;
			}
		}
	}
	throw "Find Memory Properties Failed.";
	return physicalMemoryProperties.memoryTypeCount;
}
