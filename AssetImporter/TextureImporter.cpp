#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "TextureImporter.h"
#include "ImportTextureData.h"

TextureImporter::TextureImporter()
{
}

TextureImporter::~TextureImporter()
{
}

bool TextureImporter::Load2DTexture(const std::string & SrcFileName, const std::string & TarFileName)
{
	int w, h, n;
	unsigned char *data = stbi_load(SrcFileName.c_str(), &w, &h, &n, 0);
	ImportAsset* ImportAsset = new ImportTextureData();
	ImportTextureData* ImportTexture = static_cast<ImportTextureData*>(ImportAsset);
	if (n == 3)
	{
		ImportTexture->TextureData.resize(w * h * 4);
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				size_t dstIndex = i * w * 4 + j * 4;
				size_t srcIndex = i * w * 3 + j * 3;
				ImportTexture->TextureData[dstIndex] = data[srcIndex];
				ImportTexture->TextureData[dstIndex + 1] = data[srcIndex + 1];
				ImportTexture->TextureData[dstIndex + 2] = data[srcIndex + 2];
				ImportTexture->TextureData[dstIndex + 3] = 1.0f;
			}
		}
		ImportTexture->TextureChannel = 4;
	}
	else
	{
		ImportTexture->TextureData.resize(w * h * n);
		memcpy(ImportTexture->TextureData.data(), data, w * h * n);
		ImportTexture->TextureChannel = n;
	}
	ImportTexture->TextureType = TextureTypeEnum::ImportTexture2D;
	ImportTexture->TextureWidth = w;
	ImportTexture->TextureHeight = h;
	ImportTexture->TextureDepth = 1;
	stbi_image_free(data);
	ImportTexture->Serialize(TarFileName);
	return true;
}
