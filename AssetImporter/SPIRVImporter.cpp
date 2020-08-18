#include "SPIRVImporter.h"

SPIRVWord LoadSPIRVWord(char*& pData)
{
	SPIRVWord word;
	memcpy(word.bytes, pData, sizeof(char) * 4);
	pData += 4;
	return word;
}

SPIRVImporter::SPIRVImporter()
{
}

SPIRVImporter::~SPIRVImporter()
{
}

bool SPIRVImporter::LoadSPIRVShader(const std::string& SrcFileName, const std::string& TarFileName)
{
	FileHelper fileHelper(SrcFileName, FileHelper::FileMode::FileRead);
	std::vector<char> buffer;
	fileHelper.ReadAllBinaryFile(buffer);
	GetSPIRVShaderParams(buffer);
	return true;
}

void SPIRVImporter::GetSPIRVShaderParams(std::vector<char>& data)
{
	int totalWordCount = data.size() / 4;
	char* pData = data.data();
	SPIRVWord word0 = LoadSPIRVWord(pData);
	SPIRVWord magicNumber;
	magicNumber.wordValue = 0x07230203;
	SPIRVWord zeroNumber = { 0x0, 0x0, 0x0, 0x0 };
	
	assert(magicNumber == word0);
	SPIRVWord word1 = LoadSPIRVWord(pData);
	SPIRVWord word2 = LoadSPIRVWord(pData);
	SPIRVWord word3 = LoadSPIRVWord(pData);
	int bound = word3.wordValue;
	SPIRVWord word4 = LoadSPIRVWord(pData);
	assert(zeroNumber == word4);

	int curWordCount = 5;
	while (curWordCount < totalWordCount)
	{
		SPIRVWord instructionHead = LoadSPIRVWord(pData);
		short instructionWordCount = instructionHead.shortValue[1];
		short instructionWordOp = instructionHead.shortValue[0];
		
		std::vector<SPIRVWord> operandVec;
		operandVec.reserve(instructionWordCount - 1);
		for (int i = 0; i < instructionWordCount - 1; i++)
		{
			operandVec.push_back(LoadSPIRVWord(pData));
		}
		ProcessInstruction(instructionWordOp, operandVec);
		curWordCount += instructionWordCount;
	}
}

void SPIRVImporter::ProcessInstruction(short op, const std::vector<SPIRVWord>& operands)
{
	switch (op)
	{
		case 5:
			ProcessOpName(operands);
			break;
		case 6:
			ProcessOpMemberName(operands);
			break;
		case 15:
			ProcessEntryPoint(operands);
			break;
		case 32:
			ProcessOpTypePointer(operands);
			break;
		case 59:
			ProcessOpVariable(operands);
			break;
		case 71:
			ProcessOpDecorate(operands);
			break;
	}
}

void SPIRVImporter::ProcessEntryPoint(const std::vector<SPIRVWord>& operands)
{
	if (operands[0].wordValue == 0)
		mSPIRVGlobalState.EntryPoint.shaderType = ShaderTypeEnum::ImportVertexShader;
	else if (operands[0].wordValue == 4)
		mSPIRVGlobalState.EntryPoint.shaderType = ShaderTypeEnum::ImportFragmentShader;
	else if (operands[0].wordValue == 5)
		mSPIRVGlobalState.EntryPoint.shaderType = ShaderTypeEnum::ImportComputeShader;
	mSPIRVGlobalState.EntryPoint.entryPointId = operands[1].wordValue;
	int strIndex = 2;
	mSPIRVGlobalState.EntryPoint.name = LoadString(operands, strIndex);
	for (int j = strIndex + 1; j < operands.size(); j++)
	{
		mSPIRVGlobalState.EntryPoint.opId.push_back(operands[j].wordValue);
	}
}

void SPIRVImporter::ProcessOpName(const std::vector<SPIRVWord>& operands)
{
	SPIRV_OpName opName;
	int strIndex = 1;
	opName.id = operands[0].wordValue;
	opName.name = LoadString(operands, strIndex);
	mSPIRVGlobalState.OpNames[opName.id] = opName;
}

void SPIRVImporter::ProcessOpMemberName(const std::vector<SPIRVWord>& operands)
{
	SPIRV_OpMemberName memberName;
	memberName.parentId = operands[0].wordValue;
	memberName.localIndex = operands[1].wordValue;
	int strIndex = 2;
	memberName.name = LoadString(operands, strIndex);
	mSPIRVGlobalState.OpMemberNames.push_back(memberName);
}

void SPIRVImporter::ProcessOpTypePointer(const std::vector<SPIRVWord>& operands)
{
	int id = operands[0].wordValue;
	int storage = operands[1].wordValue;
	SPIRV_OpTypePointer typePointer;
	typePointer.id = id;
	typePointer.typeId = operands[2].wordValue;
	if (storage == 2)
	{
		mSPIRVGlobalState.OpUniformBufferParamTypePointer[id] = typePointer;
	}
	else if (storage == 9)
	{
		mSPIRVGlobalState.OpUniformPushConstantBufferParamPointer[id] = typePointer;
	}
	else if (storage == 11)
	{
		mSPIRVGlobalState.OpTextureTypePointer[id] = typePointer;
	}
}

void SPIRVImporter::ProcessOpDecorate(const std::vector<SPIRVWord>& operands)
{
	int id = operands[0].wordValue;
	int decoration = operands[1].wordValue;

	if (decoration == 34)
	{
		SPIRV_OpDescriptorDecorate descDeco;
		descDeco.id = id;
		descDeco.descIndex = operands[2].wordValue;
		mSPIRVGlobalState.OpDescDecorate[id] = descDeco;
	}
	else if (decoration == 33)
	{
		SPIRV_OpBindingDecorate bindDeco;
		bindDeco.id = id;
		bindDeco.bindIndex = operands[2].wordValue;
		mSPIRVGlobalState.OpBindingDecorate[id] = bindDeco;
	}
}

void SPIRVImporter::ProcessOpVariable(const std::vector<SPIRVWord>& operands)
{
	SPIRV_OpVariable opVariable;
	opVariable.id = operands[1].wordValue;
	opVariable.typePointerId = operands[0].wordValue;
	opVariable.variableStorage = operands[2].wordValue;
	mSPIRVGlobalState.OpVariable[opVariable.id] = opVariable;
}

std::string SPIRVImporter::LoadString(const std::vector<SPIRVWord>& operands, int& index)
{
	std::string res = "";
	int i = index;
	for (; i < operands.size() && operands[i].wordValue != 0; i++)
	{
		res.push_back(operands[i].bytes[0]);
		res.push_back(operands[i].bytes[1]);
		res.push_back(operands[i].bytes[2]);
		res.push_back(operands[i].bytes[3]);
	}
	index = i;
	return res;
}
