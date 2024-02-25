#pragma once

#include "EVMDisasm.h"
#include "EVMTypes.h"
#include <chrono>
#include <vector>
#include <inttypes.h>
#include <iostream>
#include <ranges>
#include <stack>
#include <thread>

enum class EVMEmuStatus
{
	SUCCESS,
	EXECUTION_ERROR,
	FETCH_ERROR
};

using registerIntegerType = int64_t;

class EVMEmu
{
private:
	
	struct EVMContext
	{
		std::vector<int64_t> registers;
		size_t ip;
		std::stack<size_t> callStack;
	};
	
	std::vector<EVMInstruction> m_instructions;
	std::vector<uint8_t> m_memory {};
	
	EVMDisasm& m_disasm;
	std::string m_binaryFilePath {};
	EVMContext mainThreadContext {};
	
	bool m_running {};
	bool m_verbose {};
	
	std::optional<EVMInstruction> fetchInstruction();
	bool executeInstruction(const EVMInstruction& instruction);
	std::optional<registerIntegerType> readIntegerFromAddress(registerIntegerType address, MemoryAccessSize size);
	std::optional<registerIntegerType> getDataAccess(const DataAccess& da, const std::vector<registerIntegerType>& registers);
	bool saveDataAccess(registerIntegerType val, const DataAccess& da, std::vector<registerIntegerType>& registers, std::vector<uint8_t>& memory);
	void printCrashInfo (const EVMInstruction& instruction);
	
	bool mov (const EVMInstruction& instruction);
	bool loadConst (const EVMInstruction& instruction);
	bool performArithmeticOperation(const EVMInstruction& instruction);
	bool compare (const EVMInstruction& instruction);
	std::optional<size_t> jump (const EVMInstruction& instruction);
	std::optional<size_t> jumpEqual (const EVMInstruction& instruction);
	bool read (const EVMInstruction& instruction);
	bool write (const EVMInstruction& instruction);
	bool consoleRead (const EVMInstruction& instruction);
	bool consoleWrite (const EVMInstruction& instruction);
	bool createThread (const EVMInstruction& instruction);
	bool joinThread (const EVMInstruction& instruction);
	bool hlt (const EVMInstruction& instruction);
	bool sleep (const EVMInstruction& instruction);
	std::optional<size_t> call (const EVMInstruction& instruction);
	std::optional<size_t> ret (const EVMInstruction& instruction);
	bool lock (const EVMInstruction& instruction);
	bool unlock (const EVMInstruction& instruction);
	
public:
	EVMEmu(std::vector<EVMInstruction>& instructions, uint32_t dataSize, std::vector<std::byte>& initialDataBytes, uint32_t defaultStackSize, EVMDisasm& disasm, std::string binaryFile, bool verbose);
	EVMEmuStatus run();
};
