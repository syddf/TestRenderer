#include "AssetImporter.h"
#include  <direct.h>  
#include  <stdio.h> 

int main(int argc, char** argv)
{
	/*
	std::string tpath1 = "D:\\crytek-sponza-noflag\\sponza.obj";
	std::string tpath2 = "D:\\crytek-sponza-noflag\\res\\sponza.data";
	AssetImporter imp;
	imp.ImportAsset(tpath1, tpath2);
	char buffer[256];
	getcwd(buffer, 256);
	std::string path = buffer;
//	std::string srcPath = path + "/../../Asset/Src/red.png";
//	std::string dstPath = path + "/../../Asset/Dst/red.data";
	*/
	
	/*
	char buffer[256];
	getcwd(buffer, 256);
	std::string path = buffer;
	std::string srcPath = path + "/../../Asset/anisoMipMapVolumeComp.spv";
	std::string dstPath = path + "/../../Asset/anisoMipMapVolumeComp.data";
	AssetImporter imp;
	imp.ImportAsset(srcPath, dstPath);
	*/
	
	assert(argc == 3);
	std::string srcFile = argv[1];
	std::string dstFile = argv[2];
	AssetImporter importer;
	importer.ImportAsset(srcFile, dstFile);
	
	return 0;
}