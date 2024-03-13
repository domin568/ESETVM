#pragma once
#include "CLIArgParser.h"
#include "EVMDisasm.h"
#include "EVMExecutionUnit.h"
#include "EVMFile.h"
#include "EVMTypes.h"
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

class ESETVM
{
private:
	static const size_t Register_Count = 16;
	static const size_t Stack_Size = 10000;
	static const unsigned int Data_HexDump_Width = 40;
	std::string m_inputPath;
	std::string m_outputPath;
	EVMFile m_file {};
	EVMDisasm m_disasm {};
	bool m_verbose {};
	
	bool writeSourceCode();

public:
	ESETVM(std::string inputPath, std::string outputPath, bool verbose);
	[[nodiscard]] ESETVMStatus init();
	[[nodiscard]] ESETVMStatus saveSourceCode ();
	[[nodiscard]] ESETVMStatus run (const std::string& binaryFile, std::optional<size_t> maxEmulatedInstructionCount = std::nullopt);
};
