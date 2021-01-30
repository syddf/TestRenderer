#pragma once

#include "./../Source/Prefix.h"
#include "ImportSPIRVShaderData.h"

struct SPIRVWord
{
	union
	{
		char bytes[4];
		short shortValue[2];
		unsigned int wordValue;
	};
	friend bool operator == (const SPIRVWord& word1, const SPIRVWord& word2)
	{
		return word1.bytes[0] == word2.bytes[0] && word1.bytes[1] == word2.bytes[1] && word1.bytes[2] == word2.bytes[2] && word1.bytes[3] == word2.bytes[3];
	}
};


struct SPIRV_OpName
{
	int id;
	std::string name;
};

struct SPIRV_OpMemberName
{
	int parentId;
	int localIndex;
	std::string name;
};

struct SPIRV_OpTypePointer
{
	int id;
	int typeId;
};

struct SPIRV_EntryPoint
{
	ShaderTypeEnum shaderType;
	int entryPointId;
	std::string name;
	std::vector<int> opId;
};

struct SPIRV_OpDescriptorDecorate
{
	int id;
	int descIndex;
};

struct SPIRV_OpBindingDecorate
{
	int id;
	int bindIndex;
};

struct SPIRV_OpLocationDecorate
{
	int id;
	int location;
};

struct SPIRV_OpVariable
{
	int id;
	int typePointerId;
	int variableStorage;
};

enum SPIRV_TypeEnum
{
	TE_Vector,
	TE_Float,
	TE_Int,
	TE_Bool,
	TE_Matrix,
	TE_Image,
	TE_Sampler,
	TE_SamplerImage,
	TE_Array,
	TE_Struct
};

struct SPIRV_OpType
{
	int parentType = -1;
	int width;
	int dimension;
	int sampleTypeId;
	bool arrayed;
	bool combineSampled;
	int typeSize = -1;
	SPIRV_TypeEnum typeEnum;
	std::vector<int> memberTypeId;
};

struct SPIRV_OpMemberOffsetDecorate
{
	int member;
	int offset;
	int parentId;
};

struct SPIRVGlobalState
{
	SPIRV_EntryPoint EntryPoint;
	std::map<int, SPIRV_OpName> OpNames;
	std::vector<SPIRV_OpMemberName> OpMemberNames;
	std::vector<SPIRV_OpMemberOffsetDecorate> OpMemberOffsetDecorate;
	std::map<int, SPIRV_OpTypePointer> OpTypePointer;
	std::map<int, SPIRV_OpBindingDecorate> OpBindingDecorate;
	std::map<int, SPIRV_OpDescriptorDecorate> OpDescDecorate;
	std::map<int, SPIRV_OpLocationDecorate> OpLocationDecorate;
	std::map<int, SPIRV_OpVariable> OpVariable;
	std::map<int, SPIRV_OpType> OpType;
	std::map<int, int> OpIntConstant;

	int PushConstantVariableIndex;
};

class SPIRVImporter
{
public:
	SPIRVImporter();
	~SPIRVImporter();

	bool LoadSPIRVShader(const std::string& SrcFileName, const std::string& TarFileName);

private:
	ImportAsset* GetSPIRVShaderParams(std::vector<char>& data);
	void ProcessInstruction(short op, const std::vector<SPIRVWord>& operands);
	void ProcessEntryPoint(const std::vector<SPIRVWord>& operands);
	void ProcessOpName(const std::vector<SPIRVWord>& operands);
	void ProcessOpMemberName(const std::vector<SPIRVWord>& operands);
	void ProcessOpTypePointer(const std::vector<SPIRVWord>& operands);
	void ProcessOpDecorate(const std::vector<SPIRVWord>& operands);
	void ProcessOpVariable(const std::vector<SPIRVWord>& operands);
	void ProcessOpType(short op, const std::vector<SPIRVWord>& operands);
	void ProcessOpMemberDecorate(const std::vector<SPIRVWord>& operands);
	std::string LoadString(const std::vector<SPIRVWord>& operands, int& index);
	int GetMemberOffset(int structId, int member);
	std::string GetMemberName(int structId, int member);
	std::string GetFormatName(int typeId);
	void ExportAllBlock(ImportAsset* ImportAsset);
	void ExportAllInput(ImportAsset* ImportAsset);
	void ExportAllPushConstant(ImportAsset* ImportAsset);
	void ExportStructShaderParameters(int structId, int offset, std::vector<ShaderParameter>& paramsVec, const std::string& namePrefix);
	void ExportArrayShaderParameters(int structId, int offset, std::vector<ShaderParameter>& paramsVec, const std::string& namePrefix);
	void ExportValueShaderParameters(int structId, int offset, std::vector<ShaderParameter>& paramsVec, const std::string& namePrefix);
	void ExportImageShaderParameters(int structId, int offset, std::vector<ShaderParameter>& paramsVec, const std::string& namePrefix, bool combined);

	int ArrayPaddingSize(int srcSize)
	{
		return (srcSize + 15) / 16 * 16;
	}

private:
	SPIRVGlobalState mSPIRVGlobalState;	
};