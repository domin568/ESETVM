#pragma once

#include "ESETVM.h"
#include "EVMDisasm.h"
#include "EVMTypes.h"
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <inttypes.h>
#include <stack>
#include <thread>
#include <vector>

using registerIntegerType = int64_t;

class EVMExecutionUnit
{
private:
	static std::mutex printCrashMutex;
	static std::mutex writeMemoryMutex;
	static std::mutex writeFileMutex;
	static std::mutex consoleReadMutex;
	static std::mutex consoleWriteMutex;
	static std::mutex verboseMutex;
	static std::mutex muticesMutex;
	static std::mutex interruptMutex;
	static std::atomic<bool> interrupt;
	
	std::mutex unlockMutex {};
	std::mutex joinMutex {};
	
	std::optional<size_t> m_maxEmulatedInstructionCount{};
	std::atomic<size_t>& m_emulatedInstructionCount;
	
	EVMContext m_threadContext;
	
	bool m_running {};
	bool m_verbose {};
	
	const std::vector<EVMInstruction>& m_instructions;
	std::vector<uint8_t>& m_memory;
	const EVMDisasm& m_disasm;
	const std::string& m_binaryFilePath {};
	
	std::unordered_map<registerIntegerType, std::thread> m_threads {};
	std::unordered_map<registerIntegerType, std::shared_ptr<std::mutex>>& m_mutices;
	std::set<std::shared_ptr<std::mutex>> m_currentOwnedMutices {};
	
	std::optional<std::reference_wrapper<const EVMInstruction>> fetchInstruction();
	bool executeInstruction(const EVMInstruction& instruction);
	std::optional<registerIntegerType> readIntegerFromAddress(size_t address, MemoryAccessSize size);
	std::optional<registerIntegerType> getDataAccess(const DataAccess& da, const std::vector<registerIntegerType>& registers);
	bool saveDataAccess(registerIntegerType val, const DataAccess& da, std::vector<registerIntegerType>& registers, std::vector<uint8_t>& memory);
	void printCrashInfo ();
	
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
	bool sleep (const EVMInstruction& instruction);
	std::optional<size_t> call (const EVMInstruction& instruction);
	std::optional<size_t> ret ();
	bool lock (const EVMInstruction& instruction);
	bool unlock (const EVMInstruction& instruction);
	
public:
	EVMExecutionUnit(const std::vector<EVMInstruction>& instructions, std::vector<uint8_t>& memory, const EVMDisasm& disasm, EVMContext context, std::unordered_map<registerIntegerType, std::shared_ptr<std::mutex>>& mutices, const std::string& binaryFile, bool verbose, std::optional<size_t> maxEmulatedInstructionCount, std::atomic<size_t>& emulatedInstructionCount);
	~EVMExecutionUnit();
	ESETVMStatus run();
};

