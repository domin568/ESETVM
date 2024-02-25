#include "EVMDisasm.h"

EVMDisasm::EVMDisasm(const std::vector<std::byte>& input)
{
	init(input);
}
void EVMDisasm::init(const std::vector<std::byte>& input)
{
	m_bitStreamReader.init(input);
}
EVMOpcode EVMDisasm::checkOpcode(uint8_t opcodeSize)
{
	if (const auto readVarResult = m_bitStreamReader.readVar<bitSequenceInteger>(opcodeSize, false, true); readVarResult.has_value())
	{
		bitSequenceInteger opcodeVal = readVarResult.value();
		for (const auto& it : m_opcodeBitsequences.at(opcodeSize - 3))
		{
			if (it.first == opcodeVal)
			{
				if (!m_bitStreamReader.seek(opcodeSize))
				{
					return EVMOpcode::UNKNOWN;
				}
				return it.second;
			}
		}
	}
	return EVMOpcode::UNKNOWN;
}
EVMOpcode EVMDisasm::getOpcode()
{
	// opcodes are 3-6 bits long, iterate through these sizes
	// opcodeBitsequences contain mappings for these opcodes but its index start from 0

	for (int opcodeSize = 3; opcodeSize < m_opcodeBitsequences.size() + 3; opcodeSize++)
	{
		EVMOpcode opcode = checkOpcode(opcodeSize); 
		if (opcode != EVMOpcode::UNKNOWN)
		{
			return opcode;
		}
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
			if (const auto accessTypeResult = m_bitStreamReader.readVar<bitSequenceInteger>(1); accessTypeResult.has_value())
			{
				bitSequenceInteger accessType = accessTypeResult.value();
				argument.data.dataAccess.type = DataAccessType::REGISTER;
				if (accessType == 1)
				{
					if (const auto memoryAccessSizeResult = m_bitStreamReader.readVar<bitSequenceInteger>(2); memoryAccessSizeResult.has_value())
					{
						bitSequenceInteger memoryAccessSize = memoryAccessSizeResult.value();
						argument.data.dataAccess.accessSize = m_bitStreamToMemoryAccessSize.at(memoryAccessSize);
						argument.data.dataAccess.type = DataAccessType::DEREFERENCE;
					}
					else
					{
						return std::nullopt;
					}
				}
				if (const auto registerIndexResult = m_bitStreamReader.readVar<bitSequenceInteger>(4); registerIndexResult.has_value())
				{
					bitSequenceInteger registerIndex = registerIndexResult.value();
					argument.data.dataAccess.registerIndex = registerIndex;
				}
				else
				{
					return std::nullopt;
				}
			}
			else
			{
				return std::nullopt;
			}
		}
		else if (arg == ArgumentType::CONSTANT)
		{
			if (const auto constantResult = m_bitStreamReader.readVar<int64_t>(); constantResult.has_value())
			{
				int64_t constant = constantResult.value();
				argument.data.constant = constant;
			}
			else
			{
				return std::nullopt;
			}
		}
		else if (arg == ArgumentType::ADDRESS)
		{
			if (const auto codeAddressResult = m_bitStreamReader.readVar<uint32_t>(); codeAddressResult.has_value())
			{
				uint32_t codeAddress = codeAddressResult.value();
				argument.data.codeAddress = codeAddress;
				m_labelOffsets.insert(codeAddress);
			}
			else
			{
				return std::nullopt;
			}
		}
		arguments.push_back(argument);
	}
	return arguments;
}
std::optional<std::vector<EVMInstruction>> EVMDisasm::parseInstructions()
{
    std::vector<EVMInstruction> instructions;
	size_t streamSize = m_bitStreamReader.getStreamSize();
	while (m_bitStreamReader.getStreamPosition() < streamSize)
	{
		size_t bitsLeft = streamSize - m_bitStreamReader.getStreamPosition();
		if (bitsLeft < 8)
		{
			// removal of ambiguous mov instruction at the end of bit stream
			if (const auto lastBitsResult = m_bitStreamReader.readVar<bitSequenceInteger>(bitsLeft, false); lastBitsResult.has_value())
			{
				bitSequenceInteger lastBitsInLastByte = lastBitsResult.value();
				if (lastBitsInLastByte == 0)
				{
					break;
				}
			}
			else
			{
				return std::nullopt;
			}
		}
		EVMInstruction currentInstruction{};
		currentInstruction.offset = static_cast<uint32_t>(m_bitStreamReader.getStreamPosition());
		m_codeOffsetToInstructionNum.insert({currentInstruction.offset, m_currentInstructionNum});
		EVMOpcode opcode = getOpcode();
		if (opcode == EVMOpcode::UNKNOWN)
		{
			m_error = EVMDisasmStatus::OPCODE_PARSING_ERROR;
            return std::nullopt;
		}
		currentInstruction.opcode = opcode;
		const std::vector<ArgumentType>& argumentLayout = m_opcodeArguments.at(opcode);
		if (argumentLayout.size() > 0)
		{
            std::vector<EVMArgument> arguments;
			if (const auto argumentResult = readArguments(argumentLayout); argumentResult.has_value())
			{
                arguments = argumentResult.value();
            }
            else
			{
				m_error = EVMDisasmStatus::OPCODE_ARGUMENT_PARSING_ERROR;
                return std::nullopt;
			}
			currentInstruction.arguments = arguments;
		}
		m_currentInstructionNum++;
		instructions.push_back(currentInstruction);
	}
	m_instructions = instructions;
	return instructions;
}
std::optional<std::vector<std::string>> EVMDisasm::convertInstructionsToSourceCode(std::vector<EVMInstruction>& instructions, bool labels)
{
	std::vector<std::string> sourceCodeLines {};
	for (const auto& it : instructions)
	{
		std::stringstream ss;
		if (labels)
		{
			if (const auto findIt = m_labelOffsets.find(it.offset); findIt != m_labelOffsets.cend())
			{
				ss << "sub_" << std::hex << it.offset << ":";
				ss << std::dec;
				sourceCodeLines.push_back(ss.str());
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
				ss << std::hex << "0x" << argument.data.constant;
				ss << std::dec;
			}
			else if (argument.type == ArgumentType::ADDRESS)
			{
				ss << "sub_" << std::hex << argument.data.codeAddress;
				ss << std::dec;
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
		sourceCodeLines.push_back(ss.str());
	}
	m_sourceCodeLines = sourceCodeLines;
	return sourceCodeLines;
}
std::optional<size_t> EVMDisasm::insNumFromCodeOff(uint32_t codeOffset)
{
	if (!m_codeOffsetToInstructionNum.contains(codeOffset))
	{
		return std::nullopt;
	}
	return m_codeOffsetToInstructionNum.at(codeOffset);
}
std::optional<std::string> EVMDisasm::getSourceCodeLineForIp (size_t ip)
{
	if (ip >= m_sourceCodeLines.size())
	{
		return std::nullopt;
	}
	return m_sourceCodeLines.at(ip);
}
