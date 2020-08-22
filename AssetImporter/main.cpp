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

	std::string srcPath = "D:\\Compile\\shaderVert.spv";
	std::string dstPath = "D:\\Compile\\shaderVert.data";
	std::string srcPath2 = "D:\\Compile\\shaderFrag.spv";
	std::string dstPath2 = "D:\\Compile\\shaderFrag.data";
	AssetImporter importer;
	importer.ImportAsset(srcPath, dstPath);
	importer.ImportAsset(srcPath2, dstPath2);

	return 0;
}