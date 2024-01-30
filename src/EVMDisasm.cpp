#include "EVMDisasm.h"

EVMDisasm::EVMDisasm(std::string bitStream):
	m_bitStream(bitStream),
	m_error(EVMDisasmStatus::SUCCESS)
{}

EVMOpcode EVMDisasm::checkOpcode(uint8_t opcodeSize)
{
	std::string bits = m_bitStream.readBits(opcodeSize, false);
	for (const auto& it : opcodeBitsequences[opcodeSize - 3])
	{
		if (it.first == bits)
		{
			if (!m_bitStream.seek(opcodeSize))
			{
				return EVMOpcode::UNKNOWN;
			}
			return it.second;
		}
	}
	return EVMOpcode::UNKNOWN;
}
EVMOpcode EVMDisasm::getOpcode()
{
	// opcodes are 3-6 bits long, iterate through these sizes
	// opcodeBitsequences contain mappings for these opcodes but its index start from 0

	for (int opcodeSize = 3; opcodeSize < opcodeBitsequences.size() + 3; opcodeSize++) 	
	{
		EVMOpcode opcode = checkOpcode(opcodeSize); 
		if (opcode != EVMOpcode::UNKNOWN)
		{
			return opcode;
		}
	}
	return EVMOpcode::UNKNOWN;
}
std::optional<std::vector<EVMArgument>> EVMDisasm::readArguments(std::string argumentLayout)
{
    std::vector<EVMArgument> arguments;
	for (const auto& arg : argumentLayout)
	{
		EVMArgument argument{};
		if (arg == 'R')
		{
			//accessType == 0 XXXX, read XXXX as little endian register index
			//accessType == 1 SS XXXX, decode SS as memory access size, read XXXX as little endian register index, 
			std::string accessType = m_bitStream.readBits(1);
			if (accessType == "")
			{
				return std::nullopt;
			}
			std::string registerIndexStr{}; // big endian encoding
			argument.data.dataAccess.type = 'r';
			if (accessType == "1") 
			{
				std::string memoryAccessSize = m_bitStream.readBits(2);
				if (memoryAccessSize == "")
				{
                    return std::nullopt;
				}
				argument.data.dataAccess.accessSize = bitStreamToMemoryAccessSize.at(memoryAccessSize);
				argument.data.dataAccess.type = 'd';
			}
			uint8_t registerIndex{};
			if (!m_bitStream.readVar<uint8_t>(registerIndex, 4))
			{
				return std::nullopt;
			}
			argument.type = 'R';
			argument.data.dataAccess.registerIndex = registerIndex;
		}
		else if (arg == 'C')
		{
			int64_t constant{};
			if (!m_bitStream.readVar<int64_t>(constant))
			{
				return std::nullopt;
			}
			argument.type = 'C';
			argument.data.constant = constant;
		}
		else if (arg == 'L')
		{
			uint32_t codeAddress{};
			if (!m_bitStream.readVar<uint32_t>(codeAddress))
			{
				return std::nullopt;
			}
			argument.type = 'L';
			argument.data.codeAddress = codeAddress;
			labelOffsets.push_back(codeAddress);
		}
		arguments.push_back(argument);
	}
	return arguments;
}
std::optional<std::vector<EVMInstruction>> EVMDisasm::parseInstructions()
{
    std::vector<EVMInstruction> instructions;
	size_t streamSize = m_bitStream.getStreamSize();
	while (m_bitStream.getStreamPosition() < streamSize)
	{
		size_t bitsLeft = streamSize - m_bitStream.getStreamPosition();
		if (bitsLeft < 8)
		{
			// removal of ambiguous mov instruction at the end of bit stream
			std::string lastBitsInLastByte = m_bitStream.readBits(bitsLeft, false); 
			if (lastBitsInLastByte.find("1") == std::string::npos) 
			{
				break;
			}
		}
		EVMInstruction currentInstruction{};
		currentInstruction.offset = m_bitStream.getStreamPosition();
		EVMOpcode opcode = getOpcode();
		if (opcode == EVMOpcode::UNKNOWN)
		{
			m_error = EVMDisasmStatus::OPCODE_PARSING_ERROR;
            return std::nullopt;
		}
		currentInstruction.opcode = opcode;
		std::string argumentLayout = opcodeArguments.at(opcode);
		if (argumentLayout.length() > 0)
		{
            std::vector<EVMArgument> arguments;
            const auto result = readArguments(argumentLayout);
            if (result.has_value())
            {
                arguments = result.value();
            }
            else
			{
				m_error = EVMDisasmStatus::OPCODE_ARGUMENT_PARSING_ERROR;
                return std::nullopt;
			}
			currentInstruction.arguments = arguments;
		}
		instructions.push_back(currentInstruction);
	}
	return instructions;
}
std::optional<std::vector<std::string>> EVMDisasm::convertInstructionsToSourceCode(std::vector<EVMInstruction>& instructions)
{
    std::vector<std::string> sourceCodeLines;
	for (const auto& it : instructions)
	{
		std::stringstream ss;
		const auto labelIt = std::find(labelOffsets.begin(), labelOffsets.end(), it.offset);
		if (labelIt != labelOffsets.end())
		{
			ss << "sub_" << std::hex << it.offset << ":";
			ss << std::dec;
			sourceCodeLines.push_back(ss.str());
			std::stringstream().swap(ss); // empty ss
		}
		
		ss << opcodeToName.at(it.opcode);
		if (it.arguments.size() > 0)
		{
			ss << " ";
		}
		for (const auto& argument : it.arguments)
		{
			if (argument.type == 'C')
			{
				ss << std::hex << "0x" << argument.data.constant;
				ss << std::dec;
			}
			else if (argument.type == 'L')
			{
				ss << "sub_" << std::hex << argument.data.codeAddress;
				ss << std::dec;
			}
			else if (argument.type == 'R')
			{
				if (argument.data.dataAccess.type == 'r')
				{
					ss << "r" << static_cast<int>(argument.data.dataAccess.registerIndex);
				}
				else if (argument.data.dataAccess.type == 'd')
				{
					ss << memoryAccessSizeToName.at(argument.data.dataAccess.accessSize) << "[" << "r" << static_cast<int>(argument.data.dataAccess.registerIndex) << "]";
				}
			}
			if (&argument != &it.arguments.back())
			{
				ss << ", ";
			}
		}
		sourceCodeLines.push_back(ss.str());
	}
	return sourceCodeLines;
}
