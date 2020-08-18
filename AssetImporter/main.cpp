#include "AssetImporter.h"
#include  <direct.h>  
#include  <stdio.h> 

int main(int argc, char** argv)
{
	char buffer[256];
	getcwd(buffer, 256);
	std::string path = buffer;
//	std::string srcPath = path + "/../../Asset/Src/red.png";
//	std::string dstPath = path + "/../../Asset/Dst/red.data";
	std::string srcPath = "D:\\Compile\\shader.spv";
	std::string dstPath = "D:\\Compile\\shader.data";
	AssetImporter importer;
	importer.ImportAsset(srcPath, dstPath);
	return 0;
}