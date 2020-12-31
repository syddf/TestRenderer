#include "AssetImporter.h"
#include  <direct.h>  
#include  <stdio.h> 

int main(int argc, char** argv)
{
	std::string tpath1 = "C:\\Users\\syddfyuan\\Downloads\\VCTRenderer-master\\VCTRenderer-master\\engine\\assets\\models\\crytek-sponza\\sponza.obj";
	std::string tpath2 = "C:\\Users\\syddfyuan\\Downloads\\VCTRenderer-master\\VCTRenderer-master\\engine\\assets\\models\\crytek-sponza\\res\\sponza.data";
	AssetImporter imp;
	imp.ImportAsset(tpath1, tpath2);
	char buffer[256];
	getcwd(buffer, 256);
	std::string path = buffer;
//	std::string srcPath = path + "/../../Asset/Src/red.png";
//	std::string dstPath = path + "/../../Asset/Dst/red.data";
	assert(argc == 3);
	std::string srcFile = argv[1];
	std::string dstFile = argv[2];
	AssetImporter importer;
	importer.ImportAsset(srcFile, dstFile);

	return 0;
}