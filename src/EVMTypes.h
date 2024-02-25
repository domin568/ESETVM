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

using bitSequenceInteger = uint8_t;
