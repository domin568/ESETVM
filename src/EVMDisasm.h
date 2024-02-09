#pragma once
#include "utils.h"
#include "bitStreamReader.h"
#include <inttypes.h>
#include <vector>
#include <map>
#include <sstream>
#include <optional>

enum class EVMDisasmStatus
{
	SUCCESS,
	OPCODE_PARSING_ERROR,
	OPCODE_ARGUMENT_PARSING_ERROR,
	INSTRUCTIONS_TO_SOURCE_CODE_ERROR
};

enum class EVMOpcode
{
	UNKNOWN,
	MOV,
	LOADCONST,
	ADD,
	SUB,
	DIV,
	MOD,
	MUL,
	COMPARE,
	JUMP,
	JUMPEQUAL,
	READ,
	WRITE,
	CONSOLEREAD,
	CONSOLEWRITE,
	CREATETHREAD,
	JOINTHREAD,
	HLT,
	SLEEP,
	CALL,
	RET,
	LOCK,
	UNLOCK
};
enum class MemoryAccessSize
{
	NONE,
	BYTE,
	WORD,
	DWORD,
	QWORD
};
struct DataAccess
{
	char type; // 'r' for register and 'd' for dereference
	MemoryAccessSize accessSize;
	uint8_t registerIndex;
};
struct EVMArgument
{
	char type; // R for register data access, L for code offset, C for constant
	union
	{
		uint32_t codeAddress;
		int64_t constant;
		DataAccess dataAccess;
	}data;
};
struct EVMInstruction
{
	EVMOpcode opcode;
	uint64_t offset; // in file
	std::vector<EVMArgument> arguments;
};

using bitSequenceInteger = uint8_t;

class EVMDisasm
{
private:
	BitStreamReader m_bitStreamReader {};
	EVMDisasmStatus m_error {EVMDisasmStatus::SUCCESS};

	std::vector<EVMInstruction> m_instructions{};
	std::vector<std::string> m_sourceCodeLines{};
	std::vector<uint32_t> m_labelOffsets{};

	// total 21 opcodes
	const std::vector<std::map<bitSequenceInteger, EVMOpcode>> m_opcodeBitsequences =
	{
		{
			// 2 * 3 bits
			{0b000,EVMOpcode::MOV},
			{0b001,EVMOpcode::LOADCONST}
		},
		{
			// 4 * 4 bits
			{0b1100,EVMOpcode::CALL},
			{0b1101,EVMOpcode::RET},
			{0b1110,EVMOpcode::LOCK},
			{0b1111,EVMOpcode::UNLOCK},
		},
		{
			// 10 * 5 bits
			{0b01100,EVMOpcode::COMPARE},
			{0b01101,EVMOpcode::JUMP},
			{0b01110,EVMOpcode::JUMPEQUAL},
			{0b10000,EVMOpcode::READ},
			{0b10001,EVMOpcode::WRITE},
			{0b10010,EVMOpcode::CONSOLEREAD},
			{0b10011,EVMOpcode::CONSOLEWRITE},
			{0b10100,EVMOpcode::CREATETHREAD},
			{0b10101,EVMOpcode::JOINTHREAD},
			{0b10110,EVMOpcode::HLT},
			{0b10111,EVMOpcode::SLEEP},
		},
		{
			// 5 * 6 bits
			{0b010001,EVMOpcode::ADD},
			{0b010010,EVMOpcode::SUB},
			{0b010011,EVMOpcode::DIV},
			{0b010100,EVMOpcode::MOD},
			{0b010101,EVMOpcode::MUL}
		}
	};
	const std::map<EVMOpcode, std::string> m_opcodeArguments =
	{
		{EVMOpcode::MOV, "RR"},
		{EVMOpcode::LOADCONST, "CR"},
		{EVMOpcode::CALL, "L"},
		{EVMOpcode::RET, ""},
		{EVMOpcode::LOCK, "R"},
		{EVMOpcode::UNLOCK, "R"},
		{EVMOpcode::COMPARE, "RRR"},
		{EVMOpcode::JUMP, "L"},
		{EVMOpcode::JUMPEQUAL, "LRR"},
		{EVMOpcode::READ, "RRRR"},
		{EVMOpcode::WRITE, "RRR"},
		{EVMOpcode::CONSOLEREAD, "R"},
		{EVMOpcode::CONSOLEWRITE, "R"},
		{EVMOpcode::CREATETHREAD, "LR"},
		{EVMOpcode::JOINTHREAD, "R"},
		{EVMOpcode::HLT, ""},
		{EVMOpcode::SLEEP, "R"},
		{EVMOpcode::ADD, "RRR"},
		{EVMOpcode::SUB, "RRR"},
		{EVMOpcode::DIV, "RRR"},
		{EVMOpcode::MOD, "RRR"},
		{EVMOpcode::MUL, "RRR"}
	};
	const std::map<bitSequenceInteger, MemoryAccessSize> m_bitStreamToMemoryAccessSize =
	{
		{0b00, MemoryAccessSize::BYTE},
		{0b01, MemoryAccessSize::WORD},
		{0b10, MemoryAccessSize::DWORD},
		{0b11, MemoryAccessSize::QWORD}
	};
	const std::map<EVMOpcode, std::string> m_opcodeToName =
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
	const std::map<MemoryAccessSize, std::string> m_memoryAccessSizeToName =
	{
		{MemoryAccessSize::BYTE, "byte"},
		{MemoryAccessSize::WORD, "word"},
		{MemoryAccessSize::DWORD, "dword"},
		{MemoryAccessSize::QWORD, "qword"}
	};
	EVMOpcode checkOpcode(uint8_t opcodeSize);
	EVMOpcode getOpcode();
	std::optional<std::vector<EVMArgument>> readArguments(std::string argumentLayout);
	
public:
	EVMDisasm(){};
	EVMDisasm(const std::vector<std::byte>& input);
	void init(const std::vector<std::byte>& input);
	EVMDisasmStatus getError() const { return m_error; };
	std::optional<std::vector<EVMInstruction>> parseInstructions();
	std::optional<std::vector<std::string>> convertInstructionsToSourceCode(std::vector<EVMInstruction>& instructions);
	std::vector<std::string> getSourceCodeLines() { return m_sourceCodeLines; }
};

