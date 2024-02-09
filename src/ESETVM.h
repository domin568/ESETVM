#pragma once
#include "CLIArgParser.h"
#include "EVMDisasm.h"
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
	SOURCE_CODE_WRITE_ERROR
};

class ESETVM
{
private:
	bool saveSourceCode(std::vector<std::string>& sourceCodeLines);
	void showHelp();
	
	std::string m_inputPath;
	std::string m_outputPath;
	std::vector<EVMInstruction> m_instructions {};
	EVMFile m_file {};
	EVMDisasm m_disasm {};

public:
	ESETVM(std::string inputPath, std::string m_outputPath);
	[[nodiscard]] ESETVMStatus init();
	[[nodiscard]] ESETVMStatus disassemble ();
	[[nodiscard]] ESETVMStatus run ();
};
