#include "EVMEmu.h"

EVMEmu::EVMEmu(std::vector<EVMInstruction>& instructions, uint32_t dataSize, std::vector<std::byte>& initialDataBytes, uint32_t defaultStackSize, EVMDisasm& disasm, std::string binaryPath, bool verbose):
m_instructions(instructions),
m_disasm(disasm),
m_binaryFilePath(binaryPath),
m_running(true),
m_verbose(verbose)
{
	m_memory.resize(dataSize);
	if (initialDataBytes.size() > 0)
	{
		std::copy(initialDataBytes.begin(), initialDataBytes.end(),
				  reinterpret_cast<std::byte*>(m_memory.data()));
	}
	mainThreadContext.registers.resize(16);
}
std::optional<EVMInstruction> EVMEmu::fetchInstruction()
{
	if (mainThreadContext.ip <= m_instructions.size())
	{
		if (m_verbose)
		{
			const auto srcLine = m_disasm.getSourceCodeLineForIp(mainThreadContext.ip);
			if (!srcLine.has_value())
			{
				return std::nullopt;
			}
			for (size_t i = 0; i < mainThreadContext.callStack.size(); i++)
			{
				std::cerr << "\t";
			}
			std::cerr << srcLine.value() << std::endl;
		}
		return m_instructions.at(mainThreadContext.ip);
	}
	return std::nullopt;
}
EVMEmuStatus EVMEmu::run()
{
	while (m_running)
	{
		const auto instructionResult = fetchInstruction();
		if (!instructionResult.has_value())
		{
			return EVMEmuStatus::FETCH_ERROR;
		}
		if (!executeInstruction(instructionResult.value()))
		{
			printCrashInfo(instructionResult.value());
			return EVMEmuStatus::EXECUTION_ERROR;
		}
	}
	return EVMEmuStatus::SUCCESS;
}
std::optional<registerIntegerType> EVMEmu::readIntegerFromAddress(const registerIntegerType address, const MemoryAccessSize size)
{
	if (address >= m_memory.size())
	{
		return std::nullopt;
	}
	std::vector<uint8_t> tmp {m_memory.begin() + address, m_memory.begin() + address + static_cast<unsigned int>(size)};
	return utils::convertToInteger<registerIntegerType>(tmp);
}
std::optional<registerIntegerType> EVMEmu::getDataAccess(const DataAccess& da, const std::vector<registerIntegerType>& registers)
{
	registerIntegerType regVal {};
	if (da.registerIndex >= registers.size())
	{
		return std::nullopt;
	}
	regVal = registers[da.registerIndex];
	
	if (da.type == DataAccessType::REGISTER)
	{
		return regVal;
	}
	else if (da.type == DataAccessType::DEREFERENCE)
	{
		if (const auto dereferenceResult = readIntegerFromAddress(regVal, da.accessSize); dereferenceResult.has_value())
		{
			return dereferenceResult.value();
		}
	}
	return std::nullopt;
}
bool EVMEmu::saveDataAccess(registerIntegerType val, const DataAccess& da, std::vector<registerIntegerType>& registers, std::vector<uint8_t>& memory)
{
	if (da.registerIndex < registers.size())
	{
		registerIntegerType& regVal = registers.at(da.registerIndex);
		if (da.type == DataAccessType::REGISTER)
		{
			regVal = val;
			return true;
		}
		else if (da.type == DataAccessType::DEREFERENCE)
		{
			size_t accessSize = static_cast<size_t>(da.accessSize);
			if (regVal >= memory.size() - accessSize)
			{
				return false;
			}
			std::vector<uint8_t> data = utils::convertIntegerToBytes(val, accessSize);
			std::reverse(data.begin(), data.end()); // BE -> LE
			std::copy(data.cbegin(), data.cend(), memory.begin() + regVal);
			return true;
		}
	}
	return false;
}
void EVMEmu::printCrashInfo (const EVMInstruction& instruction)
{
	std::cerr << "Program crashed at instruction: ";
	const auto srcLine = m_disasm.getSourceCodeLineForIp(mainThreadContext.ip);
	if (!srcLine.has_value())
	{
		std::cerr << "unknown" << std::endl;
	}
	std::cerr << srcLine.value() << std::endl;
	for (size_t regIter = 0; regIter <mainThreadContext.registers.size(); regIter++)
	{
		std::cerr << "R" << regIter << "= " << std::hex << std::setfill('0') << std::setw(sizeof(registerIntegerType) * 2) << mainThreadContext.registers.at(regIter) << std::endl;
		std::cerr << std::dec;
	}
}
bool EVMEmu::executeInstruction(const EVMInstruction& instruction)
{
	size_t nextIns = mainThreadContext.ip + 1;
	switch (instruction.opcode)
	{
		case EVMOpcode::MOV:
		{
			if (!mov(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::LOADCONST:
		{
			if (!loadConst(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::ADD:
		case EVMOpcode::SUB:
		case EVMOpcode::DIV:
		case EVMOpcode::MOD:
		case EVMOpcode::MUL:
		{
			if (!performArithmeticOperation(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::COMPARE:
		{
			if (!compare(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::JUMP:
		{
			const auto jumpResult = jump(instruction);
			if (!jumpResult.has_value())
			{
				return false;
			}
			nextIns = jumpResult.value();
			break;
		}
		case EVMOpcode::JUMPEQUAL:
		{
			const auto jeResult = jumpEqual(instruction);
			if (!jeResult.has_value())
			{
				return false;
			}
			nextIns = jeResult.value();
			break;
		}
		case EVMOpcode::READ:
		{
			if (!read(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::WRITE:
		{
			if (!write(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::CONSOLEREAD:
		{
			if (!consoleRead(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::CONSOLEWRITE:
		{
			if (!consoleWrite(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::CREATETHREAD:
		{
			if (!createThread(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::JOINTHREAD:
		{
			if (!joinThread(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::HLT:
		{
			m_running = false;
			return true;
		}
		case EVMOpcode::SLEEP:
		{
			if (!sleep(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::CALL:
		{
			const auto callResult = call(instruction);
			if (!callResult.has_value())
			{
				return false;
			}
			nextIns = callResult.value();
			break;
		}
		case EVMOpcode::RET:
		{
			const auto retResult = ret(instruction);
			if (!retResult.has_value())
			{
				return false;
			}
			nextIns = retResult.value();
			break;
		}
		case EVMOpcode::LOCK:
		{
			if (!lock(instruction))
			{
				return false;
			}
			break;
		}
		case EVMOpcode::UNLOCK:
		{
			if (!unlock(instruction))
			{
				return false;
			}
			break;
		}
		default:
		{
			break;
		}
	}
	mainThreadContext.ip = nextIns;
	return true;
}
bool EVMEmu::mov(const EVMInstruction& instruction)
{
	const DataAccess& daArg1 = instruction.arguments.at(0).data.dataAccess;
	const DataAccess& daArg2 = instruction.arguments.at(1).data.dataAccess;
	
	const auto arg1Result = getDataAccess(daArg1, mainThreadContext.registers);
	if (!arg1Result.has_value())
	{
		return false;
	}
	if (!saveDataAccess(arg1Result.value(), daArg2, mainThreadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMEmu::loadConst(const EVMInstruction& instruction)
{
	/*
	if (instruction.arguments.size() != 2 ||
		instruction.arguments.at(0).type != ArgumentType::CONSTANT ||
		instruction.arguments.at(1).type != ArgumentType::DATA_ACCESS)
	{
		return false;
	}
	 */
	const DataAccess& da = instruction.arguments.at(1).data.dataAccess;
	const registerIntegerType& val = instruction.arguments.at(0).data.constant;
	if (!saveDataAccess(val, da, mainThreadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMEmu::performArithmeticOperation(const EVMInstruction& instruction)
{
	/*
	if (!std::all_of(instruction.arguments.cbegin(), instruction.arguments.cend(), [](EVMArgument arg) { return arg.type == ArgumentType::DATA_ACCESS; }))
	{
		return false;
	}
	*/
	// use pointers to make it faster?
	std::vector<DataAccess> daArgs;
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	
	//if (daArgs.size() == 3)
	registerIntegerType arg0 {};
	const auto arg0Result = getDataAccess(daArgs.at(0), mainThreadContext.registers);
	if (!arg0Result.has_value())
	{
		return false;
	}
	arg0 = arg0Result.value();
			
	registerIntegerType arg1 {};
	const auto arg1Result = getDataAccess(daArgs.at(1), mainThreadContext.registers);
	if (!arg1Result.has_value())
	{
		return false;
	}
	arg1 = arg1Result.value();

	registerIntegerType acc;
	switch(instruction.opcode)
	{
		case EVMOpcode::ADD:
			acc = arg0 + arg1;
			break;
		case EVMOpcode::SUB:
			acc = arg0 - arg1;
			break;
		case EVMOpcode::DIV:
			acc = arg0 / arg1;
			break;
		case EVMOpcode::MOD:
			acc = arg0 % arg1;
			break;
		case EVMOpcode::MUL:
			acc = arg0 * arg1;
			break;
		default:
			return false;
			break;
	}
	return saveDataAccess(acc, daArgs.at(2), mainThreadContext.registers, m_memory);
}
bool EVMEmu::compare (const EVMInstruction& instruction)
{
	/*
	if (!std::all_of(instruction.arguments.cbegin(), instruction.arguments.cend(), [](EVMArgument& arg){ return arg.type == ArgumentType::DATA_ACCESS; }))
	{
		return false;
	}
	*/
	std::vector<DataAccess> daArgs;
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	const auto arg1Result = getDataAccess(daArgs.at(0), mainThreadContext.registers);
	const auto arg2Result = getDataAccess(daArgs.at(1), mainThreadContext.registers);
	if ((!arg1Result.has_value()) || (!arg2Result.has_value()))
	{
		return false;
	}
	if (arg1Result.value() == arg2Result.value())
	{
		if (!saveDataAccess(0, daArgs.at(2), mainThreadContext.registers, m_memory))
		{
			return false;
		}
	}
	else if (arg1Result.value() < arg2Result.value())
	{
		if (!saveDataAccess(-1, daArgs.at(2), mainThreadContext.registers, m_memory))
		{
			return false;
		}
	}
	else if (arg1Result.value() > arg2Result.value())
	{
		if (!saveDataAccess(1, daArgs.at(2), mainThreadContext.registers, m_memory))
		{
			return false;
		}
	}
	return true;
}
std::optional<size_t> EVMEmu::jump(const EVMInstruction& instruction)
{
	uint32_t codeOffset = instruction.arguments.at(0).data.codeAddress;
	const auto insNum = m_disasm.insNumFromCodeOff(codeOffset);
	if (!insNum.has_value())
	{
		return std::nullopt;
	}
	if (insNum.value() >= m_instructions.size())
	{
		return std::nullopt;
	}
	return insNum.value();
}
std::optional<size_t> EVMEmu::jumpEqual(const EVMInstruction& instruction)
{
	DataAccess daArg1 = instruction.arguments.at(1).data.dataAccess;
	DataAccess daArg2 = instruction.arguments.at(2).data.dataAccess;
	const auto arg1Result = getDataAccess(daArg1, mainThreadContext.registers);
	const auto arg2Result = getDataAccess(daArg2, mainThreadContext.registers);
	if ((!arg1Result.has_value()) || (!arg2Result.has_value()))
	{
		return std::nullopt;
	}
	if (arg1Result.value() == arg2Result.value())
	{
		const auto jumpResult = jump(instruction);
		if (!jumpResult.has_value())
		{
			return std::nullopt;
		}
		return jumpResult.value();
	}
	return mainThreadContext.ip + 1;
}
bool EVMEmu::read (const EVMInstruction& instruction)
{
	std::vector<DataAccess> daArgs {};
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	
	const auto arg1 = getDataAccess(daArgs.at(0), mainThreadContext.registers); // offset in input file
	const auto arg2 = getDataAccess(daArgs.at(1), mainThreadContext.registers); // number of bytes to read
	const auto arg3 = getDataAccess(daArgs.at(2), mainThreadContext.registers); // memory address to which read bytes will be stored
	
	if ((!arg1.has_value()) || (!arg2.has_value()) || (!arg3.has_value()))
	{
		return false;
	}
	if (m_binaryFilePath.size() == 0)
	{
		return false;
	}
	std::ifstream inputFile {m_binaryFilePath, std::ios::binary};
	if (!inputFile.is_open())
	{
		inputFile.close();
		return false;
	}
	inputFile.seekg(arg1.value());
	if (inputFile.bad())
	{
		inputFile.close();
		return false;
	}
	if (arg2.value() >= m_memory.size() - arg3.value())
	{
		inputFile.close();
		return false;
	}
	inputFile.read(reinterpret_cast<char*>(m_memory.data() + arg3.value()), arg2.value());
	if (inputFile.bad())
	{
		inputFile.close();
		return false;
	}
	if (!saveDataAccess(inputFile.gcount(), daArgs.at(3), mainThreadContext.registers, m_memory))
	{
		inputFile.close();
		return false;
	}
	inputFile.close();
	return true;
}
bool EVMEmu::write (const EVMInstruction& instruction)
{
	std::vector<DataAccess> daArgs {};
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	const auto arg1 = getDataAccess(daArgs.at(0), mainThreadContext.registers); // offset in output file
	const auto arg2 = getDataAccess(daArgs.at(1), mainThreadContext.registers); // number of bytes to write
	const auto arg3 = getDataAccess(daArgs.at(2), mainThreadContext.registers); // memory address from which bytes will be written
	
	if ((!arg1.has_value()) || (!arg2.has_value()) || (arg3.has_value()))
	{
		return false;
	}
	if (m_binaryFilePath.size() == 0)
	{
		return false;
	}
	std::ofstream outputFile {m_binaryFilePath, std::ios::binary};
	if (!outputFile.is_open())
	{
		outputFile.close();
		return false;
	}
	outputFile.seekp(arg1.value()); // should write zeros if beyond filesize
	if (outputFile.bad())
	{
		outputFile.close();
		return false;
	}
	if (arg2.value() >= m_memory.size() - arg3.value())
	{
		outputFile.close();
		return false;
	}
	outputFile.write(reinterpret_cast<char*>(m_memory.data() + arg3.value()), arg2.value());
	if (outputFile.bad())
	{
		outputFile.close();
		return false;
	}
	return true;
}
bool EVMEmu::consoleRead(const EVMInstruction& instruction)
{
	registerIntegerType val {};
	std::cin >> std::hex >> val;
	if (!saveDataAccess(val, instruction.arguments.at(0).data.dataAccess, mainThreadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMEmu::consoleWrite(const EVMInstruction& instruction)
{
	/*
	 if (instruction.arguments.size() != 1 && instruction.arguments[0].type != ArgumentType::DATA_ACCESS)
	 {
	 return false;
	 }
	 */
	const DataAccess& da = instruction.arguments.at(0).data.dataAccess;
	const auto daResult = getDataAccess(da, mainThreadContext.registers);
	if (!daResult.has_value())
	{
		return false;
	}
	registerIntegerType val = daResult.value();
	std::cout << std::hex << std::setfill('0') << std::setw(sizeof(val) * 2) << daResult.value() << std::endl;
	return true;
}
bool EVMEmu::createThread(const EVMInstruction& instruction)
{
	return false;
}
bool EVMEmu::joinThread(const EVMInstruction& instruction)
{
	return false;
}
bool EVMEmu::hlt(const EVMInstruction& instruction)
{
	return false;
}
bool EVMEmu::sleep(const EVMInstruction& instruction)
{
	const DataAccess& da = instruction.arguments.at(0).data.dataAccess;
	const auto sleepDuration = getDataAccess(da, mainThreadContext.registers);
	if (!sleepDuration.has_value())
	{
		return false;
	}
	std::this_thread::sleep_for (std::chrono::seconds(sleepDuration.value()));
	return true;
}
std::optional<size_t> EVMEmu::call(const EVMInstruction &instruction)
{
	if (mainThreadContext.ip + 1 >= m_instructions.size())
	{
		return std::nullopt;
	}
	const auto jumpResult = jump(instruction);
	if (!jumpResult.has_value())
	{
		return std::nullopt;
	}
	mainThreadContext.callStack.push(mainThreadContext.ip + 1);
	return jumpResult.value();
}
std::optional<size_t> EVMEmu::ret(const EVMInstruction &instruction)
{
	size_t retInsOff = mainThreadContext.callStack.top();
	mainThreadContext.callStack.pop();
	if (retInsOff >= m_instructions.size())
	{
		return std::nullopt;
	}
	return retInsOff;
}
bool EVMEmu::lock(const EVMInstruction &instruction)
{
	return false;
}
bool EVMEmu::unlock(const EVMInstruction &instruction)
{
	return false;
}
