#include "EVMDisasm.h"

// total 21 opcodes
const std::vector<std::unordered_map<bitSequenceInteger, EVMOpcode>> EVMDisasm::m_opcodeBitsequences =
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
const std::unordered_map<EVMOpcode, std::vector<ArgumentType>> EVMDisasm::m_opcodeArguments =
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
const std::unordered_map<bitSequenceInteger, MemoryAccessSize> EVMDisasm::m_bitStreamToMemoryAccessSize =
{
	{0b00, MemoryAccessSize::BYTE},
	{0b01, MemoryAccessSize::WORD},
	{0b10, MemoryAccessSize::DWORD},
	{0b11, MemoryAccessSize::QWORD}
};
const std::unordered_map<EVMOpcode, std::string> EVMDisasm::m_opcodeToName =
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
const std::unordered_map<MemoryAccessSize, std::string> EVMDisasm::m_memoryAccessSizeToName =
{
	{MemoryAccessSize::BYTE, "byte"},
	{MemoryAccessSize::WORD, "word"},
	{MemoryAccessSize::DWORD, "dword"},
	{MemoryAccessSize::QWORD, "qword"}
};

EVMDisasm::EVMDisasm(const std::vector<std::byte>& input)
{
	init(input);
}
void EVMDisasm::init(const std::vector<std::byte>& input)
{
	m_bitStreamReader.init(input);
}
EVMOpcode EVMDisasm::getOpcode()
{
	// opcodes are 3-6 bits long, iterate through these sizes
	size_t opcodeSize = 3;
	for (const auto& currentSizeIt : m_opcodeBitsequences)
	{
		const auto readVarResult = m_bitStreamReader.readVar<bitSequenceInteger>(opcodeSize, false, true);
		if (!readVarResult.has_value())
		{
			return EVMOpcode::UNKNOWN;
		}
		bitSequenceInteger opcodeVal = readVarResult.value();
		for (const auto& opcodeBitSequence : currentSizeIt)
		{
			if (opcodeBitSequence.first == opcodeVal)
			{
				if (!m_bitStreamReader.seek(opcodeSize))
				{
					return EVMOpcode::UNKNOWN;
				}
				return opcodeBitSequence.second;
			}
		}
		opcodeSize++;
	}
	return EVMOpcode::UNKNOWN;
}
std::optional<std::vector<EVMArgument>> EVMDisasm::readArguments(const std::vector<ArgumentType>& argumentLayout)
{
    std::vector<EVMArgument> arguments;
	for (const auto& arg : argumentLayout)
	{
		EVMArgument argument{};
		argument.type = arg;
		if (arg == ArgumentType::DATA_ACCESS)
		{
			//accessType == 0 XXXX, read XXXX as little endian register index
			//accessType == 1 SS XXXX, decode SS as memory access size, read XXXX as little endian register index, 
			const auto accessTypeResult = m_bitStreamReader.readVar<bitSequenceInteger>(1);
			if (!accessTypeResult.has_value())
			{
				return std::nullopt;
			}
			bitSequenceInteger accessType = accessTypeResult.value();
			argument.data.dataAccess.type = DataAccessType::REGISTER;
			if (accessType == 1)
			{
				const auto memoryAccessSizeResult = m_bitStreamReader.readVar<bitSequenceInteger>(2);
				if (!memoryAccessSizeResult.has_value())
				{
					return std::nullopt;
				}
				bitSequenceInteger memoryAccessSize = memoryAccessSizeResult.value();
				argument.data.dataAccess.accessSize = m_bitStreamToMemoryAccessSize.at(memoryAccessSize);
				argument.data.dataAccess.type = DataAccessType::DEREFERENCE;
			}
			const auto registerIndexResult = m_bitStreamReader.readVar<bitSequenceInteger>(4);
			if (!registerIndexResult.has_value())
			{
				return std::nullopt;
			}
			bitSequenceInteger registerIndex = registerIndexResult.value();
			argument.data.dataAccess.registerIndex = registerIndex;
		}
		else if (arg == ArgumentType::CONSTANT)
		{
			const auto constantResult = m_bitStreamReader.readVar<int64_t>();
			if (!constantResult.has_value())
			{
				return std::nullopt;
			}
			int64_t constant = constantResult.value();
			argument.data.constant = constant;
		}
		else if (arg == ArgumentType::ADDRESS)
		{
			const auto codeAddressResult = m_bitStreamReader.readVar<uint32_t>();
			if (!codeAddressResult.has_value())
			{
				return std::nullopt;
			}
			uint32_t codeAddress = codeAddressResult.value();
			argument.data.codeAddress = codeAddress;
			m_labelOffsets.insert(codeAddress);
		}
		arguments.push_back(argument);
	}
	return arguments;
}
bool EVMDisasm::parseInstructions()
{
	size_t streamSize = m_bitStreamReader.getStreamSize();
	while (m_bitStreamReader.getStreamPosition() < streamSize)
	{
		size_t bitsLeft = streamSize - m_bitStreamReader.getStreamPosition();
		if (bitsLeft < 8)
		{
			// removal of ambiguous mov instruction at the end of bit stream
			const auto lastBitsResult = m_bitStreamReader.readVar<bitSequenceInteger>(bitsLeft, false);
			if (!lastBitsResult.has_value())
			{
				return false;
			}
			bitSequenceInteger lastBitsInLastByte = lastBitsResult.value();
			if (lastBitsInLastByte == 0)
			{
				break;
			}
		}
		EVMInstruction currentInstruction{};
		currentInstruction.offset = static_cast<uint32_t>(m_bitStreamReader.getStreamPosition());
		m_codeOffsetToInstructionNum.insert({currentInstruction.offset, m_currentInstructionNum});
		EVMOpcode opcode = getOpcode();
		if (opcode == EVMOpcode::UNKNOWN)
		{
			m_error = ESETVMStatus::OPCODE_PARSING_ERROR;
            return false;
		}
		currentInstruction.opcode = opcode;
		const std::vector<ArgumentType>& argumentLayout = m_opcodeArguments.at(opcode);
		if (argumentLayout.size() > 0)
		{
            std::vector<EVMArgument> arguments;
			const auto argumentResult = readArguments(argumentLayout);
			if (!argumentResult.has_value())
			{
				m_error = ESETVMStatus::OPCODE_ARGUMENT_PARSING_ERROR;
				return false;
                
            }
			arguments = argumentResult.value();
			currentInstruction.arguments = arguments;
		}
		m_currentInstructionNum++;
		m_instructions.push_back(currentInstruction);
	}
	return true;
}
bool EVMDisasm::convertInstructionsToSourceCode(bool labels)
{
	m_sourceCodeLines.reserve(m_instructions.size() + m_labelOffsets.size());
	for (const auto& it : m_instructions)
	{
		std::stringstream ss;
		if (labels)
		{
			if (const auto findIt = m_labelOffsets.find(it.offset); findIt != m_labelOffsets.cend())
			{
				ss << "sub_" << std::hex << it.offset << ":" << std::dec;
				m_sourceCodeLines.push_back(ss.str());
				std::stringstream().swap(ss); // empty ss
			}
		}
		ss << m_opcodeToName.at(it.opcode);
		if (it.arguments.size() > 0)
		{
			ss << " ";
		}
		for (const auto& argument : it.arguments)
		{
			if (argument.type == ArgumentType::CONSTANT)
			{
				ss << std::hex << "0x" << argument.data.constant << std::dec;
			}
			else if (argument.type == ArgumentType::ADDRESS)
			{
				ss << "sub_" << std::hex << argument.data.codeAddress << std::dec;
			}
			else if (argument.type == ArgumentType::DATA_ACCESS)
			{
				if (argument.data.dataAccess.type == DataAccessType::REGISTER)
				{
					ss << "r" << static_cast<int>(argument.data.dataAccess.registerIndex);
				}
				else if (argument.data.dataAccess.type == DataAccessType::DEREFERENCE)
				{
					ss << m_memoryAccessSizeToName.at(argument.data.dataAccess.accessSize) << "[" << "r" << static_cast<int>(argument.data.dataAccess.registerIndex) << "]";
				}
			}
			if (&argument != &it.arguments.back())
			{
				ss << ", ";
			}
		}
		m_sourceCodeLines.push_back(ss.str());
	}
	return true;
}
std::optional<size_t> EVMDisasm::insNumFromCodeOff(uint32_t codeOffset) const
{
	if (!m_codeOffsetToInstructionNum.contains(codeOffset))
	{
		return std::nullopt;
	}
	return m_codeOffsetToInstructionNum.at(codeOffset);
}
std::optional<std::string> EVMDisasm::getSourceCodeLineForIp (size_t ip) const
{
	if (ip >= m_sourceCodeLines.size())
	{
		return std::nullopt;
	}
	return m_sourceCodeLines.at(ip);
}
