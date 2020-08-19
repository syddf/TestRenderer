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
		case 72:
			ProcessOpMemberDecorate(operands);
			break;
	}
	if (op >= 20 && op <= 30)
	{
		ProcessOpType(op, operands);
	}
	if (op == 43)
	{
		int id = operands[1].wordValue;
		int intVal = operands[2].wordValue;
		mSPIRVGlobalState.OpIntConstant[id] = intVal;
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
	mSPIRVGlobalState.OpTypePointer[id] = typePointer;
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

void SPIRVImporter::ProcessOpType(short op, const std::vector<SPIRVWord>& operands)
{
	SPIRV_OpType opType;
	if (op == 20)
	{
		int id = operands[0].wordValue;
		opType.width = 1;
		opType.typeEnum = SPIRV_TypeEnum::TE_Bool;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 21)
	{
		int id = operands[0].wordValue;
		opType.width = operands[1].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Int;
		opType.typeSize = operands[1].wordValue / 8;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 22)
	{
		int id = operands[0].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Float;
		opType.width = operands[1].wordValue;
		opType.typeSize = operands[1].wordValue / 8;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 23)
	{
		int id = operands[0].wordValue;
		opType.parentType = operands[1].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Vector;
		opType.width = operands[2].wordValue;
		opType.typeSize = opType.width * mSPIRVGlobalState.OpType[opType.parentType].typeSize;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 24)
	{
		int id = operands[0].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Matrix;
		opType.width = 1;
		opType.typeSize = 64;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 25)
	{
		int id = operands[0].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Image;
		opType.sampleTypeId = operands[1].wordValue;
		opType.dimension = operands[2].wordValue;
		opType.arrayed = operands[4].wordValue;
		opType.combineSampled = operands[5].wordValue;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 26)
	{
		int id = operands[0].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Sampler;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 27)
	{
		int id = operands[0].wordValue;
		opType.parentType = operands[1].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_SamplerImage;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 28)
	{
		int id = operands[0].wordValue;
		opType.parentType = operands[1].wordValue;
		opType.width = mSPIRVGlobalState.OpIntConstant[operands[2].wordValue];
		opType.typeSize = opType.width + ArrayPaddingSize(mSPIRVGlobalState.OpType[opType.parentType].typeSize);
		opType.typeEnum = SPIRV_TypeEnum::TE_Array;
		mSPIRVGlobalState.OpType[id] = opType;
	}
	else if (op == 30)
	{
		int id = operands[0].wordValue;
		opType.typeEnum = SPIRV_TypeEnum::TE_Struct;
		opType.width = operands.size() - 1;
		for (int i = 1; i < operands.size(); i++)
		{
			opType.memberTypeId.push_back(operands[i].wordValue);
		}
		int maxOffset = GetMemberOffset(id, opType.memberTypeId.size() - 1);
		int lastChildSize = mSPIRVGlobalState.OpType[opType.memberTypeId.back()].typeSize;
		opType.typeSize = ArrayPaddingSize(maxOffset + lastChildSize);
		mSPIRVGlobalState.OpType[id] = opType;
	}
}

void SPIRVImporter::ProcessOpMemberDecorate(const std::vector<SPIRVWord>& operands)
{
	int structureType = operands[0].wordValue;
	int member = operands[1].wordValue;
	int decoration = operands[2].wordValue;
	if (decoration == 35)
	{
		int val = operands[3].wordValue;
		SPIRV_OpMemberOffsetDecorate decorate;
		decorate.parentId = structureType;
		decorate.member = member;
		decorate.offset = val;
		mSPIRVGlobalState.OpMemberOffsetDecorate.push_back(decorate);
	}
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

int SPIRVImporter::GetMemberOffset(int structId, int member)
{
	for (auto mo : mSPIRVGlobalState.OpMemberOffsetDecorate)
	{
		if (mo.member == member && mo.parentId == structId) return mo.offset;
	}
	return 0;
}

void SPIRVImporter::CalcAllTypeOffset()
{
}

void SPIRVImporter::CalcAllBlockSize()
{
	for (auto iter = mSPIRVGlobalState.OpType.begin(); iter != mSPIRVGlobalState.OpType.end(); iter++)
	{

	}
}

void SPIRVImporter::ExportShaderParameters()
{
}
