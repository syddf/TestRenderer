#pragma once

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