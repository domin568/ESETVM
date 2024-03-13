#include "ESETVM.h"

ESETVM::ESETVM(std::string inputPath, std::string outputPath, bool verbose):
m_inputPath(inputPath),
m_outputPath(outputPath),
m_file(m_inputPath),
m_verbose(verbose)
{}

ESETVMStatus ESETVM::init()
{
	if (m_file.getError() != ESETVMStatus::SUCCESS)
	{
		std::cerr << "Input file error" << std::endl;
		return m_file.getError();
	}
	std::vector<std::byte> codeBytes = m_file.getCodeBytes();
	m_disasm.init(codeBytes);
	if (!m_disasm.parseInstructions())
	{
		std::cerr << "Instruction parsing error" << std::endl;
		return m_disasm.getError();
	}
	return ESETVMStatus::SUCCESS;
}
bool ESETVM::writeSourceCode()
{
	std::ofstream outputFile(m_outputPath);
	outputFile << ".dataSize " << m_file.getDataSize() << std::endl;
	if (m_file.getInitialDataSize() > 0)
	{
		outputFile << ".data" << std::endl << std::endl;
		std::vector<std::byte> dataBytes = m_file.getDataBytes();
		outputFile << utils::byteArrayToHexString(dataBytes, Data_HexDump_Width);
		outputFile << std::endl << std::endl;
	}
	outputFile << ".code" << std::endl << std::endl;
	for (const auto& line : m_disasm.getSourceCodeLines())
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
ESETVMStatus ESETVM::saveSourceCode()
{
	std::vector<std::string> sourceCodeLines{};
	if (!m_disasm.convertInstructionsToSourceCode())
	{
		std::cerr << "Source code produce error" << std::endl;
		return ESETVMStatus::PRODUCE_SOURCE_CODE_ERROR;
	}	
	if (!writeSourceCode())
	{
		std::cerr << "Source code writing error" << std::endl;
		return ESETVMStatus::SOURCE_CODE_WRITE_ERROR;
	}
	return ESETVMStatus::SUCCESS;
}
ESETVMStatus ESETVM::run(const std::string& binaryFile, std::optional<size_t> maxEmulatedInstructionCount)
{
	const std::vector<std::byte>& initialDataBytes = m_file.getDataBytes();
	std::vector<uint8_t> memory {};
	memory.resize(m_file.getDataSize());
	if (initialDataBytes.size() > 0)
	{
		std::copy(initialDataBytes.begin(), initialDataBytes.end(),
				  reinterpret_cast<std::byte*>(memory.data()));
	}
	EVMContext mainThreadContext {Register_Count, Stack_Size};
	std::unordered_map<registerIntegerType, std::shared_ptr<std::mutex>> mutices {};
	m_disasm.convertInstructionsToSourceCode(false);
	
	std::atomic<size_t> instructionCounter {};
	EVMExecutionUnit mainThread {m_disasm.getInstructions(), memory, m_disasm, mainThreadContext, mutices, binaryFile, m_verbose, maxEmulatedInstructionCount, instructionCounter};
	ESETVMStatus status = mainThread.run();
	return status;
}
