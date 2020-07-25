#pragma once
#include "Prefix.h"
#include "svpng.h"

class OutputImageHelper
{
public:
	static void OutputPNG(std::string OutputPath, std::vector<UInt8>& Pixels, int Width, int Height)
	{
		FILE* fp = fopen(OutputPath.c_str(), "wb");
		svpng(fp, Width, Height, Pixels.data(), 1);
		fclose(fp);
	}
};