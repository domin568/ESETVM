#pragma once
#include "BitStreamReader.h"
#include "EVMTypes.h"
#include "utils.h"
#include <inttypes.h>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <vector>

class EVMDisasm
{
private:
	BitStreamReader m_bitStreamReader {};
	ESETVMStatus m_error {ESETVMStatus::SUCCESS};

	std::vector<EVMInstruction> m_instructions {};
	std::vector<std::string> m_sourceCodeLines {};
	std::set<uint32_t> m_labelOffsets {};
	std::unordered_map<uint32_t, size_t> m_codeOffsetToInstructionNum {};
	size_t m_currentInstructionNum {};

	// total 21 opcodes
	const std::vector<std::unordered_map<bitSequenceInteger, EVMOpcode>> m_opcodeBitsequences =
	{
		{
			// 2 * 3 bits
			{0b000, EVMOpcode::MOV},
			{0b001, EVMOpcode::LOADCONST}
		},
		{
			// 4 * 4 bits
			{0b1100, EVMOpcode::CALL},
			{0b1101, EVMOpcode::RET},
			{0b1110, EVMOpcode::LOCK},
			{0b1111, EVMOpcode::UNLOCK},
		},
		{
			// 10 * 5 bits
			{0b01100, EVMOpcode::COMPARE},
			{0b01101, EVMOpcode::JUMP},
			{0b01110, EVMOpcode::JUMPEQUAL},
			{0b10000, EVMOpcode::READ},
			{0b10001, EVMOpcode::WRITE},
			{0b10010, EVMOpcode::CONSOLEREAD},
			{0b10011, EVMOpcode::CONSOLEWRITE},
			{0b10100, EVMOpcode::CREATETHREAD},
			{0b10101, EVMOpcode::JOINTHREAD},
			{0b10110, EVMOpcode::HLT},
			{0b10111, EVMOpcode::SLEEP},
		},
		{
			// 5 * 6 bits
			{0b010001, EVMOpcode::ADD},
			{0b010010, EVMOpcode::SUB},
			{0b010011, EVMOpcode::DIV},
			{0b010100, EVMOpcode::MOD},
			{0b010101, EVMOpcode::MUL}
		}
	};
	
	const std::unordered_map<EVMOpcode, std::vector<ArgumentType>> m_opcodeArguments =
	{
		{EVMOpcode::MOV, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::LOADCONST, {ArgumentType::CONSTANT, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::CALL, {ArgumentType::ADDRESS}},
		{EVMOpcode::RET, {}},
		{EVMOpcode::LOCK, {ArgumentType::DATA_ACCESS}},
		{EVMOpcode::UNLOCK, {ArgumentType::DATA_ACCESS}},
		{EVMOpcode::COMPARE, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::JUMP, {ArgumentType::ADDRESS}},
		{EVMOpcode::JUMPEQUAL, {ArgumentType::ADDRESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::READ, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::WRITE, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::CONSOLEREAD, {ArgumentType::DATA_ACCESS}},
		{EVMOpcode::CONSOLEWRITE, {ArgumentType::DATA_ACCESS}},
		{EVMOpcode::CREATETHREAD, {ArgumentType::ADDRESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::JOINTHREAD, {ArgumentType::DATA_ACCESS}},
		{EVMOpcode::HLT, {}},
		{EVMOpcode::SLEEP, {ArgumentType::DATA_ACCESS}},
		{EVMOpcode::ADD, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::SUB, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::DIV, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::MOD, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}},
		{EVMOpcode::MUL, {ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS, ArgumentType::DATA_ACCESS}}
	};
	const std::unordered_map<bitSequenceInteger, MemoryAccessSize> m_bitStreamToMemoryAccessSize =
	{
		{0b00, MemoryAccessSize::BYTE},
		{0b01, MemoryAccessSize::WORD},
		{0b10, MemoryAccessSize::DWORD},
		{0b11, MemoryAccessSize::QWORD}
	};
	const std::unordered_map<EVMOpcode, std::string> m_opcodeToName =
	{
		{EVMOpcode::UNKNOWN, "unknown"}, 
		{EVMOpcode::MOV, "mov"},
		{EVMOpcode::LOADCONST, "loadConst"},
		{EVMOpcode::CALL, "call"},
		{EVMOpcode::RET, "ret"},
		{EVMOpcode::LOCK, "lock"},
		{EVMOpcode::UNLOCK, "unlock"},
		{EVMOpcode::COMPARE, "compare"},
		{EVMOpcode::JUMP, "jump"},
		{EVMOpcode::JUMPEQUAL, "jumpEqual"},
		{EVMOpcode::READ, "read"},
		{EVMOpcode::WRITE, "write"},
		{EVMOpcode::CONSOLEREAD, "consoleRead"},
		{EVMOpcode::CONSOLEWRITE, "consoleWrite"},
		{EVMOpcode::CREATETHREAD, "createThread"},
		{EVMOpcode::JOINTHREAD, "joinThread"},
		{EVMOpcode::HLT, "hlt"},
		{EVMOpcode::SLEEP, "sleep"},
		{EVMOpcode::ADD, "add"},
		{EVMOpcode::SUB, "sub"},
		{EVMOpcode::DIV, "div"},
		{EVMOpcode::MOD, "mod"},
		{EVMOpcode::MUL, "mul"}
	};
	const std::unordered_map<MemoryAccessSize, std::string> m_memoryAccessSizeToName =
	{
		{MemoryAccessSize::BYTE, "byte"},
		{MemoryAccessSize::WORD, "word"},
		{MemoryAccessSize::DWORD, "dword"},
		{MemoryAccessSize::QWORD, "qword"}
	};
	EVMOpcode checkOpcode(uint8_t opcodeSize);
	EVMOpcode getOpcode();
	std::optional<std::vector<EVMArgument>> readArguments(const std::vector<ArgumentType>& argumentLayout);
	
public:
	EVMDisasm() = default;
	EVMDisasm(const std::vector<std::byte>& input);
	void init(const std::vector<std::byte>& input);
	ESETVMStatus getError() const { return m_error; };
	std::optional<std::vector<EVMInstruction>> parseInstructions();
	std::optional<std::vector<std::string>> convertInstructionsToSourceCode(std::vector<EVMInstruction>& instructions, bool labels = true);
	std::vector<std::string> getSourceCodeLines() { return m_sourceCodeLines; }
	std::optional<size_t> insNumFromCodeOff(uint32_t codeOffset) const;
	std::optional<std::string> getSourceCodeLineForIp (size_t ip) const;
};

