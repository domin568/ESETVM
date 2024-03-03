#pragma once

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
enum class DataAccessPurpose
{
	READ,
	WRITE
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
	DataAccessPurpose purpose;
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
	SUCCESS,
	INPUT_FILE_PARSING_ERROR,
	DEASSEMBLE_ERROR,
	PRODUCE_SOURCE_CODE_ERROR,
	SOURCE_CODE_WRITE_ERROR,
	EMULATION_ERROR,
	EMULATION_INS_NUM_EXCEEDED,
	EXECUTION_ERROR,
	FETCH_ERROR,
};

using bitSequenceInteger = uint8_t;
