#pragma once
#include "utils.h"
#include "bitStreamReader.h"
#include <inttypes.h>
#include <vector>
#include <map>
#include <sstream>

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
enum MemoryAccessSize 
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
	uint64_t offset;
	std::vector<EVMArgument> arguments;
};

class EVMDisasm
{
private:
	bitStreamReader m_bitStream;
	EVMDisasmStatus m_error;

	std::vector<EVMInstruction> instructions{};
	std::vector<std::string> sourceCodeLines{};
	std::vector<uint32_t> labelOffsets{};

	// total 21 opcodes
	std::vector<std::map<std::string, EVMOpcode>> opcodeBitsequences =
	{
		{
			{"000",EVMOpcode::MOV},  // 2 * 3 bits
			{"001",EVMOpcode::LOADCONST}
		},
		{
			{"1100",EVMOpcode::CALL}, // 4 * 4 bits
			{"1101",EVMOpcode::RET},
			{"1110",EVMOpcode::LOCK},
			{"1111",EVMOpcode::UNLOCK},
		},
		{
			{"01100",EVMOpcode::COMPARE},  // 10 * 5 bits
			{"01101",EVMOpcode::JUMP},
			{"01110",EVMOpcode::JUMPEQUAL},
			{"10000",EVMOpcode::READ},
			{"10001",EVMOpcode::WRITE},
			{"10010",EVMOpcode::CONSOLEREAD},
			{"10011",EVMOpcode::CONSOLEWRITE},
			{"10100",EVMOpcode::CREATETHREAD},
			{"10101",EVMOpcode::JOINTHREAD},
			{"10110",EVMOpcode::HLT},
			{"10111",EVMOpcode::SLEEP},
		},
		{
			{"010001",EVMOpcode::ADD}, // 5 * 6 bits
			{"010010",EVMOpcode::SUB},
			{"010011",EVMOpcode::DIV},
			{"010100",EVMOpcode::MOD},
			{"010101",EVMOpcode::MUL}
		}
	};
	const std::map<EVMOpcode, std::string> opcodeArguments =
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
	const std::map<std::string, MemoryAccessSize> bitStreamToMemoryAccessSize =
	{
		{"00", MemoryAccessSize::BYTE},
		{"10", MemoryAccessSize::WORD},
		{"01", MemoryAccessSize::DWORD},
		{"11", MemoryAccessSize::QWORD}
	};
	const std::map<EVMOpcode, std::string> opcodeToName =
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
	const std::map<MemoryAccessSize, std::string> memoryAccessSizeToName =
	{
		{MemoryAccessSize::BYTE, "byte"},
		{MemoryAccessSize::WORD, "word"},
		{MemoryAccessSize::DWORD, "dword"},
		{MemoryAccessSize::QWORD, "qword"}
	};
	EVMOpcode checkOpcode(uint8_t opcodeSize);
	EVMOpcode getOpcode();
	bool readArguments(std::string argumentLayout, std::vector<EVMArgument>& arguments);
	
	
public:
	EVMDisasm(std::string bitStream);
	EVMDisasmStatus getError() const { return m_error; };
	bool parseInstructions(std::vector<EVMInstruction>& instructions);
	bool convertInstructionsToSourceCode(std::vector<EVMInstruction>& instructions, std::vector<std::string>& sourceCodeLines);
	std::vector<std::string> getSourceCodeLines() { return sourceCodeLines; }
};

