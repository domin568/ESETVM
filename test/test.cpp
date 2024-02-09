#include "../src/BitStreamReader.h"
#include "../src/BitStreamReader.cpp"
#include "../src/CLIArgParser.h"
#include "../src/CLIArgParser.cpp"
#include "../src/ESETVM.h"
#include "../src/EVMDisasm.h"
#include "../src/EVMDisasm.cpp"
#include "../src/EVMFile.h"
#include "../src/EVMFile.cpp"
#include "../src/utils.h"
#include "../src/utils.cpp"
#include <vector>
#include <numeric>
#include <gtest/gtest.h>

#define S(x) #x
#define STR(x) S(x)

template<typename... Ts>
std::vector<std::byte> makeBytes(Ts&&... args)
{
	return{std::byte(std::forward<Ts>(args))...};
}

static std::vector<std::byte> crcCodeTest = makeBytes(
	0x21, 0x1c, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xd0 );
static std::string crcBitStreamTest = "00100001000111001000000000000000000000000000000000000000000000000000011100111111111111111111111111111111111000000000000000000000000000000000010100100000000000000000000000000000000000000000000000000000000000000000110100100000000000000000000000000000000000000000000000000000000000000000001100110000000000000000000000000000000000000000000000000000000000000000101100100100000000000000000000000000000000000000000000000000000000000000100111010000";

static std::vector<EVMInstruction> crcInstructionsTest;

static std::vector<std::byte> crcCodeFull = makeBytes (
	0x21, 0x1c, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x80, 0xd6, 0xd0, 0x39, 0x6f, 0x00, 0x00, 0x00, 0x0d, 0x13, 0x59, 0x91, 0x7a, 0x00, 0x00, 0x19, 0x0e, 0xe0, 0x00, 0x01, 0x97, 0x24, 0x00, 0x00, 0x00, 0x13, 0x44, 0xd0, 0x2a, 0x04, 0x81, 0x10, 0x2c, 0x02, 0x80, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x13, 0x2a, 0x0b, 0x90, 0xee, 0x00, 0x00, 0x19, 0x02, 0xa0, 0x00, 0x01, 0x97, 0x24, 0x00, 0x00, 0x00, 0x05, 0x44, 0x6b, 0x1b, 0x43, 0x60, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0x72, 0x2b, 0xc0, 0x00, 0x03, 0x21, 0xdc, 0x00, 0x00, 0x32, 0xe4, 0x80, 0x00, 0x02, 0x60, 0xb6, 0x44, 0x44, 0x00, 0x00, 0x64, 0x28, 0x40, 0x00, 0x06, 0x43, 0x68, 0x00, 0x00, 0x64, 0x5e, 0x80, 0x00, 0x06, 0x41, 0x18, 0x00, 0x00, 0x64, 0x69, 0x80, 0x00, 0x06, 0x42, 0x58, 0x00, 0x00, 0x64, 0x4d, 0x80, 0x00, 0x06, 0x40, 0x38, 0x00, 0x00, 0x64, 0x3b, 0x80, 0x00, 0x02, 0x45, 0x29, 0x52, 0x21, 0x08, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x18, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x17, 0x0b, 0x70, 0x00, 0x00, 0x00, 0x8d, 0xe9, 0xe0, 0x00, 0x00, 0x70, 0x54, 0x80, 0x00, 0x02, 0x08, 0xc0, 0x11, 0x4c, 0x41, 0x0c, 0xc5, 0x05, 0x4c, 0x30, 0x4c, 0xe0, 0xa1, 0x00, 0x00, 0x05, 0x19, 0x12, 0x88, 0xad, 0x0a, 0x10, 0x00, 0x00, 0x74, 0xe8, 0x80, 0x00, 0x03, 0x04, 0x98, 0x9c, 0x26, 0xc0, 0x48, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x2a, 0x07, 0x01, 0x54, 0x39, 0x0d, 0x28, 0xe0, 0x00, 0x00, 0x05, 0x06, 0x40, 0x54, 0x00, 0x00, 0x64, 0x21, 0x40, 0x00, 0x06, 0x46, 0xe4, 0x00, 0x00, 0x64, 0x16, 0x40, 0x00, 0x06, 0x45, 0xa4, 0x00, 0x00, 0x64, 0x32, 0x40, 0x00, 0x06, 0x47, 0xc4, 0x00, 0x00, 0x64, 0x0c, 0x40, 0x00, 0x06, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x03, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x23, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x13, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x33, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x0b, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x2b, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x1b, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x3b, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x07, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x27, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x17, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x37, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x0f, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x72, 0x1c, 0x2f, 0xbe, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe0, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe8, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe4, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xec, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe2, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xea, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe6, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xee, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe1, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe9, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe5, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xed, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xe3, 0x48, 0xe4, 0x3e, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xeb, 0x48, 0xe4, 0x3e, 0x80
);

static std::vector<std::string> crcDisasmFull = {
    "loadConst 0x2710, r14",
    "loadConst 0xffffffff, r10",
    "loadConst 0x0, r11",
    "loadConst 0x0, r12",
    "loadConst 0x1, r13",
    "loadConst 0x4, r9",
    "sub_1b0:",
    "read r12, r13, r11, r0",
    "jumpEqual sub_3da, r0, r11",
    "mov byte[r11], r3",
    "call sub_bd1",
    "call sub_ee1",
    "call sub_49d",
    "mov r0, byte[r11]",
    "mov byte[r11], r0",
    "mul r0, r9, r0",
    "add r0, r13, r0",
    "mov dword[r0], r0",
    "loadConst 0x100, r1",
    "div r10, r1, r10",
    "call sub_ee1",
    "call sub_a81",
    "call sub_49d",
    "mov r0, r10",
    "add r12, r13, r12",
    "jump sub_1b0",
    "sub_3da:",
    "loadConst 0xffffffff, r11",
    "call sub_f51",
    "call sub_ee1",
    "call sub_49d",
    "consoleWrite r0",
    "hlt",
    "sub_49d:",
    "call sub_1111",
    "call sub_10a1",
    "call sub_b61",
    "call sub_bd1",
    "call sub_c41",
    "call sub_cb1",
    "call sub_d21",
    "call sub_d91",
    "call sub_e01",
    "call sub_ee1",
    "sub r10, r10, r10",
    "sub r2, r2, r2",
    "loadConst 0x2, r7",
    "loadConst 0x8000000000000000, r3",
    "mov r3, r4",
    "loadConst 0xffffffffffffffff, r8",
    "sub_714:",
    "jumpEqual sub_768, r0, r2",
    "jump sub_797",
    "sub_768:",
    "jumpEqual sub_950, r1, r2",
    "sub_797:",
    "compare r0, r2, r5",
    "compare r1, r2, r6",
    "compare r5, r8, r5",
    "compare r6, r8, r6",
    "jumpEqual sub_850, r5, r6",
    "add r10, r4, r10",
    "jump sub_850",
    "sub_850:",
    "jumpEqual sub_8b9, r3, r4",
    "div r4, r7, r4",
    "jump sub_901",
    "sub_8b9:",
    "loadConst 0x4000000000000000, r4",
    "sub_901:",
    "mul r0, r7, r0",
    "mul r1, r7, r1",
    "jump sub_714",
    "sub_950:",
    "mov r10, r0",
    "call sub_1501",
    "call sub_1421",
    "call sub_13b1",
    "call sub_1341",
    "call sub_12d1",
    "call sub_1261",
    "call sub_11f1",
    "call sub_1181",
    "ret",
    "sub_a81:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r0, qword[r14]",
    "ret",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r1, qword[r14]",
    "ret",
    "sub_b61:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r2, qword[r14]",
    "ret",
    "sub_bd1:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r3, qword[r14]",
    "ret",
    "sub_c41:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r4, qword[r14]",
    "ret",
    "sub_cb1:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r5, qword[r14]",
    "ret",
    "sub_d21:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r6, qword[r14]",
    "ret",
    "sub_d91:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r7, qword[r14]",
    "ret",
    "sub_e01:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r8, qword[r14]",
    "ret",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r9, qword[r14]",
    "ret",
    "sub_ee1:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r10, qword[r14]",
    "ret",
    "sub_f51:",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r11, qword[r14]",
    "ret",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r12, qword[r14]",
    "ret",
    "loadConst 0x8, r2",
    "add r14, r2, r14",
    "mov r13, qword[r14]",
    "ret",
    "sub_10a1:",
    "loadConst 0x8, r2",
    "mov qword[r14], r0",
    "sub r14, r2, r14",
    "ret",
    "sub_1111:",
    "loadConst 0x8, r2",
    "mov qword[r14], r1",
    "sub r14, r2, r14",
    "ret",
    "sub_1181:",
    "loadConst 0x8, r2",
    "mov qword[r14], r2",
    "sub r14, r2, r14",
    "ret",
    "sub_11f1:",
    "loadConst 0x8, r2",
    "mov qword[r14], r3",
    "sub r14, r2, r14",
    "ret",
    "sub_1261:",
    "loadConst 0x8, r2",
    "mov qword[r14], r4",
    "sub r14, r2, r14",
    "ret",
    "sub_12d1:",
    "loadConst 0x8, r2",
    "mov qword[r14], r5",
    "sub r14, r2, r14",
    "ret",
    "sub_1341:",
    "loadConst 0x8, r2",
    "mov qword[r14], r6",
    "sub r14, r2, r14",
    "ret",
    "sub_13b1:",
    "loadConst 0x8, r2",
    "mov qword[r14], r7",
    "sub r14, r2, r14",
    "ret",
    "sub_1421:",
    "loadConst 0x8, r2",
    "mov qword[r14], r8",
    "sub r14, r2, r14",
    "ret",
    "loadConst 0x8, r2",
    "mov qword[r14], r9",
    "sub r14, r2, r14",
    "ret",
    "sub_1501:",
    "loadConst 0x8, r2",
    "mov qword[r14], r10",
    "sub r14, r2, r14",
    "ret",
    "loadConst 0x8, r2",
    "mov qword[r14], r11",
    "sub r14, r2, r14",
    "ret",
    "loadConst 0x8, r2",
    "mov qword[r14], r12",
    "sub r14, r2, r14",
    "ret",
    "loadConst 0x8, r2",
    "mov qword[r14], r13",
    "sub r14, r2, r14",
    "ret"
};

static std::string testPath = STR(TEST_FOLDER);

bool areInstructionsEqual(const std::vector<EVMInstruction>& vec1, const std::vector<EVMInstruction>& vec2) 
{
	if (vec1.size() != vec2.size()) 
	{
		return false; // Different sizes, not equal
	}
	for (size_t i = 0; i < vec1.size(); ++i) 
	{
		if (vec1[i].opcode != vec2[i].opcode) 
		{
			return false; // Different opcodes, not equal
		}

		if (vec1[i].arguments.size() != vec2[i].arguments.size()) 
		{
			return false; // Different argument vector sizes, not equal
		}

		// Compare each argument
		for (size_t j = 0; j < vec1[i].arguments.size(); ++j) 
		{
			// Assuming you have appropriate comparison logic for your union and structs
			if (vec1[i].arguments[j].type != vec2[i].arguments[j].type)
			{
				return false;
			}
			if (vec1[i].arguments[j].type == 'C')
			{
				if (vec1[i].arguments[j].data.constant != vec2[i].arguments[j].data.constant)
				{
					return false;
				}
			}
			else if (vec1[i].arguments[j].type == 'L')
			{
				if (vec1[i].arguments[j].data.codeAddress != vec2[i].arguments[j].data.codeAddress)
				{
					return false;
				}
			}
			else if (vec1[i].arguments[j].type == 'R')
			{
				if (vec1[i].arguments[j].data.dataAccess.type != vec2[i].arguments[j].data.dataAccess.type)
				{
					return false;
				}
				if ((vec1[i].arguments[j].data.dataAccess.accessSize != vec2[i].arguments[j].data.dataAccess.accessSize) ||
					 vec1[i].arguments[j].data.dataAccess.registerIndex != vec2[i].arguments[j].data.dataAccess.registerIndex)
				{
					return false;
				}
			}
		}
	}
	return true; // All elements are equal
}

void initializeCrcInstructionsTest()
{
	EVMInstruction i1{}; i1.opcode = EVMOpcode::LOADCONST;
	EVMArgument const1; const1.type = 'C'; const1.data.constant = static_cast<int64_t>(10000); i1.arguments.push_back(const1);
	EVMArgument arg1; arg1.type = 'R';  arg1.data.dataAccess.type = 'r'; arg1.data.dataAccess.accessSize = MemoryAccessSize::NONE; arg1.data.dataAccess.registerIndex = 14; i1.arguments.push_back(arg1);
	crcInstructionsTest.push_back(i1);

	EVMInstruction i2{}; i2.opcode = EVMOpcode::LOADCONST;
	EVMArgument const2{}; const2.type = 'C'; const2.data.constant = static_cast<int64_t>(0xFFFFFFFF); i2.arguments.push_back(const2);
	EVMArgument arg1_1{}; arg1_1.type = 'R'; arg1_1.data.dataAccess.type = 'r'; arg1_1.data.dataAccess.accessSize = MemoryAccessSize::NONE; arg1_1.data.dataAccess.registerIndex = 10; i2.arguments.push_back(arg1_1);
	crcInstructionsTest.push_back(i2);

	EVMInstruction i3{}; i3.opcode = EVMOpcode::LOADCONST;
	EVMArgument const3{}; const3.type = 'C'; const3.data.constant = static_cast<int64_t>(0); i3.arguments.push_back(const3);
	EVMArgument arg1_2{}; arg1_2.type = 'R'; arg1_2.data.dataAccess.type = 'r'; arg1_2.data.dataAccess.accessSize = MemoryAccessSize::NONE; arg1_2.data.dataAccess.registerIndex = 11; i3.arguments.push_back(arg1_2);
	crcInstructionsTest.push_back(i3);

	EVMInstruction i4{}; i4.opcode = EVMOpcode::LOADCONST;
	EVMArgument const4{}; const4.type = 'C'; const4.data.constant = static_cast<int64_t>(0); i4.arguments.push_back(const4);
	EVMArgument arg1_3{}; arg1_3.type = 'R'; arg1_3.data.dataAccess.type = 'r'; arg1_3.data.dataAccess.accessSize = MemoryAccessSize::NONE; arg1_3.data.dataAccess.registerIndex = 12; i4.arguments.push_back(arg1_3);
	crcInstructionsTest.push_back(i4);

	EVMInstruction i5{}; i5.opcode = EVMOpcode::LOADCONST;
	EVMArgument const5{}; const5.type = 'C'; const5.data.constant = static_cast<int64_t>(1); i5.arguments.push_back(const5);
	EVMArgument arg1_4{}; arg1_4.type = 'R'; arg1_4.data.dataAccess.type = 'r'; arg1_4.data.dataAccess.accessSize = MemoryAccessSize::NONE; arg1_4.data.dataAccess.registerIndex = 13; i5.arguments.push_back(arg1_4);
	crcInstructionsTest.push_back(i5);

	EVMInstruction i6{}; i6.opcode = EVMOpcode::LOADCONST;
	EVMArgument const6{}; const6.type = 'C'; const6.data.constant = static_cast<int64_t>(4); i6.arguments.push_back(const6);
    EVMArgument arg1_5{}; arg1_5.type = 'R'; arg1_5.data.dataAccess.type = 'r'; arg1_5.data.dataAccess.accessSize = MemoryAccessSize::NONE; arg1_5.data.dataAccess.registerIndex = 9; i6.arguments.push_back(arg1_5);
	crcInstructionsTest.push_back(i6);

	EVMInstruction i7{}; i7.opcode = EVMOpcode::RET;
	crcInstructionsTest.push_back(i7);
}

TEST(InstructionParsingTest, ParsingInstruction)
{
	EVMDisasm disasm(crcCodeTest);

	initializeCrcInstructionsTest();
	std::vector<EVMInstruction> instructions;
    const auto parsingInstruction = disasm.parseInstructions();
    if (parsingInstruction.has_value())
    {
        instructions = parsingInstruction.value();
    }
	EXPECT_TRUE(parsingInstruction.has_value());
	EXPECT_TRUE(instructions.size() > 0);
	EXPECT_TRUE(areInstructionsEqual(instructions, crcInstructionsTest));

	std::vector<std::string> sourceCodeLines{};
    const auto sourceCodeTransform = disasm.convertInstructionsToSourceCode(instructions);
    if (sourceCodeTransform.has_value())
    {
        sourceCodeLines = sourceCodeTransform.value();
    }
	EXPECT_TRUE(sourceCodeTransform.has_value());
}

TEST(BitStreamReader, CrcTest)
{
	BitStreamReader bitStreamReader (crcCodeTest);
	std::vector<bool> bitStream;
	const auto bitStreamResult = bitStreamReader.readBits(crcCodeTest.size() * BITS_IN_BYTE, false);
	EXPECT_TRUE(bitStreamResult.has_value());
	bitStream = bitStreamResult.value();
	std::string bitStreamString {};
	for (const auto& bit : bitStream)
	{
		bitStreamString += (bit ? "1" : "0");
	}
	EXPECT_EQ(bitStreamString, crcBitStreamTest);
}
TEST(InstructionParsingCrcTest, Disasm)
{
	EVMDisasm disasm(crcCodeFull);
	EXPECT_EQ(disasm.getError(), EVMDisasmStatus::SUCCESS);
	std::vector<EVMInstruction> instructions;
    const auto instructionParsing = disasm.parseInstructions();
    if (instructionParsing.has_value())
    {
		instructions = instructionParsing.value();
    }
	EXPECT_TRUE(instructionParsing.has_value());
	EXPECT_TRUE(instructions.size() > 0);
	std::vector<std::string> sourceCodeLines{};
    const auto sourceCodeTransform = disasm.convertInstructionsToSourceCode(instructions);
    if (sourceCodeTransform.has_value())
    {
        sourceCodeLines = sourceCodeTransform.value();
    }
	EXPECT_TRUE(sourceCodeTransform.has_value());
	EXPECT_TRUE(std::equal(sourceCodeLines.begin(), sourceCodeLines.end(), crcDisasmFull.begin(), crcDisasmFull.end()));
}
TEST(ArgumentParsingTest, CliArguments)
{
	int argc = 2;
	const char* argv [2] {"", "-r"};
	CLIArgParser parse1 {argc, argv};
	EXPECT_FALSE(parse1.parseArguments());
	
	argc = 3;
	const char* argv2[3] {"", "-v", "-h"};
	CLIArgParser parse2 {argc, argv2};
	EXPECT_TRUE(parse2.parseArguments());
	EXPECT_TRUE(parse2.getFlags().verbose);
	EXPECT_TRUE(parse2.getFlags().help);
	
	argc = 4;
	std::string inputPath1 = testPath + "/samples/precompiled/crc.evm";
	const char* argv3[4] {"","-d", inputPath1.c_str(), "output_file.txt"};
	CLIArgParser parse3 {argc, argv3};
	EXPECT_TRUE(parse3.parseArguments());
	EXPECT_TRUE(parse3.getFlags().disassemble);
	EXPECT_EQ(parse3.getInputPath(), inputPath1);
	EXPECT_EQ(parse3.getOutpuPath(),"output_file.txt");
	
	argc = 3;
	const char* argv4[3] {"","-r","output_file.txt"};
	CLIArgParser parse4 {argc, argv4};
	EXPECT_FALSE(parse4.parseArguments());
	
	argc = 3;
	const char* argv5[] {"","-d","output_file.txt"};
	CLIArgParser parse5 {argc, argv5};
	EXPECT_FALSE(parse5.parseArguments());
}

std::vector<std::string> getAllFilesInDirectory(const std::string& directoryPath) 
{
	std::vector<std::string> files {};
	for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
	{
		if (std::filesystem::is_regular_file(entry.path()))
		{
			files.push_back(entry.path().string());
		}
	}
	return files;
}

TEST(AllSamplesTest, DeassembleAllProvidedSamples)
{
	std::string evmFilesFolder = testPath + "/samples/precompiled/";
	std::string outputFileFolder = testPath + "/samples/recompile_test/";
	if (!std::filesystem::exists(outputFileFolder))
	{
		std::filesystem::create_directories(outputFileFolder);
	}
	std::vector<std::string> evmFilePaths = getAllFilesInDirectory(evmFilesFolder);
	for (const auto& filePath : evmFilePaths)
	{
		if(filePath.ends_with(".evm"))
		{
			std::filesystem::path path(filePath);
			std::string fileName = path.stem().string();
			std::string outputFilePath = outputFileFolder + fileName + "_decompiled.easm";
			
			ESETVM evm {filePath, outputFilePath};
			EXPECT_EQ(evm.init(), ESETVMStatus::SUCCESS);
			EXPECT_EQ(evm.disassemble(), ESETVMStatus::SUCCESS);
			
			std::string recompiledPath = outputFileFolder + fileName + "_recompiled.evm";
			std::string command = "python2 " + testPath + "/compiler.py " + outputFilePath + " " + recompiledPath;
			EXPECT_FALSE(std::system(command.c_str())); // recompile samples using provided python script
						
			std::ifstream file1(filePath, std::ios::binary);
			std::ifstream file2(recompiledPath, std::ios::binary);

			EXPECT_FALSE (!file1.is_open() || !file2.is_open());
			EXPECT_FALSE (std::filesystem::file_size(filePath) != std::filesystem::file_size(recompiledPath));

			std::vector<char> content1(std::istreambuf_iterator<char>(file1), {});
			std::vector<char> content2(std::istreambuf_iterator<char>(file2), {});
			EXPECT_TRUE(std::equal(content1.begin(), content1.end(), content2.begin()));
		}
	}
}

