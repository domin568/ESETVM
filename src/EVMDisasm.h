#pragma once
#include "BitStreamReader.h"
#include "EVMTypes.h"
#include "utils.h"
#include <inttypes.h>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

class EVMDisasm
{
private:
	static const std::vector<std::unordered_map<bitSequenceInteger, EVMOpcode>> m_opcodeBitsequences;
	static const std::unordered_map<EVMOpcode, std::vector<ArgumentType>> m_opcodeArguments;
	static const std::unordered_map<bitSequenceInteger, MemoryAccessSize> m_bitStreamToMemoryAccessSize;
	static const std::unordered_map<EVMOpcode, std::string> m_opcodeToName;
	static const std::unordered_map<MemoryAccessSize, std::string> m_memoryAccessSizeToName;

	BitStreamReader m_bitStreamReader {};
	ESETVMStatus m_error {ESETVMStatus::SUCCESS};

	std::vector<EVMInstruction> m_instructions {};
	std::vector<std::string> m_sourceCodeLines {};
	std::set<uint32_t> m_labelOffsets {};
	std::unordered_map<uint32_t, size_t> m_codeOffsetToInstructionNum {};
	size_t m_currentInstructionNum {};

	EVMOpcode getOpcode();
	std::optional<std::vector<EVMArgument>> readArguments(const std::vector<ArgumentType>& argumentLayout);
	
public:
	EVMDisasm() = default;
	EVMDisasm(const std::vector<std::byte>& input);
	void init(const std::vector<std::byte>& input);
	ESETVMStatus getError() const { return m_error; };

	const std::vector<EVMInstruction>& getInstructions() const { return m_instructions; }
	const std::vector<std::string>& getSourceCode() const { return m_sourceCodeLines; }
	bool parseInstructions();
	bool convertInstructionsToSourceCode(bool labels = true);
	std::vector<std::string> getSourceCodeLines() { return m_sourceCodeLines; }
	std::optional<size_t> insNumFromCodeOff(uint32_t codeOffset) const;
	std::optional<std::string> getSourceCodeLineForIp (size_t ip) const;
};

