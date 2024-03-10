#pragma once

#include <inttypes.h>
#include <stack>
#include <vector>

using bitSequenceInteger = uint8_t;

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
	NONE = 0,
	BYTE = 1,
	WORD = 2,
	DWORD = 4,
	QWORD = 8
};
enum class DataAccessType
{
	REGISTER,
	DEREFERENCE
};
struct DataAccess
{
	DataAccessType type;
	MemoryAccessSize accessSize;
	uint8_t registerIndex;
};
enum class ArgumentType
{
	DATA_ACCESS,
	ADDRESS,
	CONSTANT
};
struct EVMArgument
{
	ArgumentType type;
	union
	{
		uint32_t codeAddress;
		int64_t constant;
		DataAccess dataAccess;
	} data;
};
struct EVMInstruction
{
	EVMOpcode opcode;
	uint32_t offset; // code adresses are 32 bits
	std::vector<EVMArgument> arguments;
};
enum class ESETVMStatus
{
	CLI_ARG_PARSING_ERROR = -1,
	SUCCESS = 0,
	DISASSEMBLE_ERROR = 1,
	PRODUCE_SOURCE_CODE_ERROR = 2,
	SOURCE_CODE_WRITE_ERROR = 3,
	EMULATION_ERROR = 4,
	EMULATION_INS_NUM_EXCEEDED = 5,
	EXECUTION_ERROR = 6,
	FETCH_ERROR = 7,
	FILE_OPEN_ERROR = 8,
	FILE_READ_ERROR = 9,
	NOT_EVM_FILE = 10,
	FILE_CORRUPTED = 11,
	FILE_TOO_BIG = 12,
	OPCODE_PARSING_ERROR = 13,
	OPCODE_ARGUMENT_PARSING_ERROR = 14,
	INSTRUCTIONS_TO_SOURCE_CODE_ERROR = 15
};
struct EVMContext
{
	const size_t stackSize;
	std::vector<int64_t> registers;
	size_t ip;
	std::stack<size_t> callStack;
	EVMContext(size_t registerCount, size_t maxStackSize): 
	registers{},
	ip{0},
	callStack{},
	stackSize{maxStackSize}
	{
		registers.resize(registerCount); 
	}
};
