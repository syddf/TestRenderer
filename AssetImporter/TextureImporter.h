#pragma once

#include "./../Source/Prefix.h"

class TextureImporter
{
public:
	TextureImporter();
	~TextureImporter();

	bool Load2DTexture(const std::string& SrcFileName, const std::string& TarFileName);
};