#include "ESETVM.h"
#include "EVMDisasm.h"
#include "EVMFile.h"

ESETVM::ESETVM(std::string inputPath, std::string outputPath):
m_inputPath(inputPath),
m_outputPath(outputPath),
m_file(m_inputPath)
{}

ESETVMStatus ESETVM::init()
{
	if (m_file.getError() != EVMFileStatus::SUCCESS)
	{
		std::cerr << "Input file error" << std::endl;
		return ESETVMStatus::INPUT_FILE_PARSING_ERROR;
	}
	std::vector<std::byte> codeBytes = m_file.getCodeBytes();
	m_disasm.init(codeBytes);
	if (const auto instructionParse = m_disasm.parseInstructions(); instructionParse.has_value())
	{
		m_instructions = std::move(instructionParse.value());
	}
	else
	{
		std::cerr << "Instruction parsing error" << std::endl;
		return ESETVMStatus::DEASSEMBLE_ERROR;
	}
	return ESETVMStatus::SUCCESS;
}
bool ESETVM::saveSourceCode(std::vector<std::string>& sourceCodeLines)
{
	std::ofstream outputFile(m_outputPath);
	outputFile << ".dataSize " << m_file.getDataSize() << std::endl;
	if (m_file.getInitialDataSize() > 0)
	{
		outputFile << ".data" << std::endl << std::endl;
		std::vector<std::byte> dataBytes = m_file.getDataBytes();
		outputFile << utils::byteArrayToHexString(dataBytes, 40);
		outputFile << std::endl << std::endl;
	}
	outputFile << ".code" << std::endl << std::endl;
	for (const auto& line : sourceCodeLines)
	{
		outputFile << line << std::endl;
	}
	if (outputFile.fail()) 
	{
		return false;
	}
	outputFile.close();
	return true;
}
ESETVMStatus ESETVM::disassemble()
{
	std::vector<std::string> sourceCodeLines{};
	if (const auto sourceCodeConvert = m_disasm.convertInstructionsToSourceCode(m_instructions); sourceCodeConvert.has_value())
	{
		sourceCodeLines = std::move(sourceCodeConvert.value());
	}
	else
	{
		std::cerr << "Source code produce error" << std::endl;
		return ESETVMStatus::PRODUCE_SOURCE_CODE_ERROR;
	}
	if (!saveSourceCode(sourceCodeLines))
	{
		std::cerr << "Source code writing error" << std::endl;
		return ESETVMStatus::SOURCE_CODE_WRITE_ERROR;
	}
	return ESETVMStatus::SUCCESS;
}
ESETVMStatus ESETVM::run()
{
	return ESETVMStatus::SUCCESS;
}
