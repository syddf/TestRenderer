#pragma once

#include "Prefix.h"

class PathHelper
{
public:
	static std::string GetPathSuffix(const std::string& Path);
};

std::string PathHelper::GetPathSuffix(const std::string& Path)
{
	auto DotPos = Path.find_last_of('.');
	if (DotPos == Path.npos)
	{
		return "";
	}
	return Path.substr(DotPos + 1);
}