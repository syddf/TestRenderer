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

struct SPIRV_OpVariable
{
	int id;
	int typePointerId;
	int variableStorage;
};

struct SPIRVGlobalState
{
	SPIRV_EntryPoint EntryPoint;
	std::map<int, SPIRV_OpName> OpNames;
	std::vector<SPIRV_OpMemberName> OpMemberNames;
	std::map<int, SPIRV_OpTypePointer> OpUniformBufferParamTypePointer;
	std::map<int, SPIRV_OpTypePointer> OpUniformPushConstantBufferParamPointer;
	std::map<int, SPIRV_OpTypePointer> OpTextureTypePointer;
	std::map<int, SPIRV_OpBindingDecorate> OpBindingDecorate;
	std::map<int, SPIRV_OpDescriptorDecorate> OpDescDecorate;

	std::map<int, SPIRV_OpVariable> OpVariable;
};

class SPIRVImporter
{
public:
	SPIRVImporter();
	~SPIRVImporter();

	bool LoadSPIRVShader(const std::string& SrcFileName, const std::string& TarFileName);

private:
	void GetSPIRVShaderParams(std::vector<char>& data);
	void ProcessInstruction(short op, const std::vector<SPIRVWord>& operands);
	void ProcessEntryPoint(const std::vector<SPIRVWord>& operands);
	void ProcessOpName(const std::vector<SPIRVWord>& operands);
	void ProcessOpMemberName(const std::vector<SPIRVWord>& operands);
	void ProcessOpTypePointer(const std::vector<SPIRVWord>& operands);
	void ProcessOpDecorate(const std::vector<SPIRVWord>& operands);
	void ProcessOpVariable(const std::vector<SPIRVWord>& operands);
	std::string LoadString(const std::vector<SPIRVWord>& operands, int& index);

private:
	SPIRVGlobalState mSPIRVGlobalState;
	
};