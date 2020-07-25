#pragma once

#include "./../Source/Prefix.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

class ModelImporter
{
public:
	ModelImporter();
	~ModelImporter();

	bool Load(const std::string& SrcFileName, const std::string& TarFileName);
};