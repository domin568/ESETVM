#pragma once
#include "CLIArgParser.h"
#include "EVMDisasm.h"
#include "EVMEmu.h"
#include "EVMFile.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <utility>

enum class ESETVMStatus
{
	SUCCESS,
	INPUT_FILE_PARSING_ERROR,
	DEASSEMBLE_ERROR,
	PRODUCE_SOURCE_CODE_ERROR,
	SOURCE_CODE_WRITE_ERROR,
	EMULATION_ERROR
};

class ESETVM
{
private:
	static constexpr size_t Default_Stack_Size = 0x1000;
	
	std::string m_inputPath;
	std::string m_outputPath;
	std::vector<EVMInstruction> m_instructions {};
	EVMFile m_file {};
	EVMDisasm m_disasm {};
	bool m_verbose {};
	
	bool writeSourceCode(std::vector<std::string>& sourceCodeLines);

public:
	ESETVM(std::string inputPath, std::string outputPath, bool verbose);
	[[nodiscard]] ESETVMStatus init();
	[[nodiscard]] ESETVMStatus saveSourceCode ();
	[[nodiscard]] ESETVMStatus run (const std::string& binaryFile);
	
	std::vector<EVMInstruction> getInstructions() const { return m_instructions; }
};
