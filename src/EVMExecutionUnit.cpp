#include "EVMExecutionUnit.h"

std::mutex EVMExecutionUnit::printCrashMutex;
std::mutex EVMExecutionUnit::writeMemoryMutex;
std::mutex EVMExecutionUnit::writeFileMutex;
std::mutex EVMExecutionUnit::consoleReadMutex;
std::mutex EVMExecutionUnit::consoleWriteMutex;
std::mutex EVMExecutionUnit::verboseMutex;
std::mutex EVMExecutionUnit::muticesMutex;
std::mutex EVMExecutionUnit::interruptMutex;
std::atomic<bool> EVMExecutionUnit::interrupt = false;

EVMExecutionUnit::EVMExecutionUnit(const std::vector<EVMInstruction>& instructions, std::vector<uint8_t>& memory, const EVMDisasm& disasm, EVMContext context, std::unordered_map<registerIntegerType, std::shared_ptr<std::mutex>>& mutices, const std::string& binaryFile, bool verbose, std::optional<size_t> maxEmulatedInstructionCount, std::atomic<size_t>& emulatedInstructionCount):
m_maxEmulatedInstructionCount(maxEmulatedInstructionCount),
m_emulatedInstructionCount(emulatedInstructionCount),
m_threadContext(context),
m_running(true),
m_verbose(verbose),
m_instructions(instructions),
m_memory(memory),
m_disasm(disasm),
m_binaryFilePath(binaryFile),
m_mutices(mutices)
{
	m_threadContext.registers.resize(16);
}
EVMExecutionUnit::~EVMExecutionUnit()
{
	{
		std::unique_lock l {unlockMutex};
		for (auto t : m_mutices)
		{
			t.second->unlock();
		}
	}
	{
		std::unique_lock l {joinMutex};
		for (auto& t : m_threads)
		{
			if (t.second.joinable())
			{
				t.second.join();
			}
		}
	}
}
std::optional<EVMInstruction> EVMExecutionUnit::fetchInstruction()
{
	if (m_threadContext.ip <= m_instructions.size())
	{
		if (m_verbose)
		{
			std::unique_lock l {verboseMutex};
			
			const auto srcLine = m_disasm.getSourceCodeLineForIp(m_threadContext.ip);
			if (!srcLine.has_value())
			{
				return std::nullopt;
			}
			for (size_t i = 0; i < m_threadContext.callStack.size(); i++)
			{
				std::cerr << "\t";
			}
			std::cerr << std::this_thread::get_id() << ": " << srcLine.value() << std::endl;
		}
		return m_instructions.at(m_threadContext.ip);
	}
	return std::nullopt;
}
ESETVMStatus EVMExecutionUnit::run()
{
	while (m_running)
	{
		const auto instructionResult = fetchInstruction();
		if (!instructionResult.has_value())
		{
			return ESETVMStatus::FETCH_ERROR;
		}
		if (!executeInstruction(instructionResult.value()))
		{
			printCrashInfo(instructionResult.value());
			return ESETVMStatus::EXECUTION_ERROR;
		}
		if (m_maxEmulatedInstructionCount.has_value())
		{
			m_emulatedInstructionCount++;
			if (m_emulatedInstructionCount > m_maxEmulatedInstructionCount.value())
			{
				return ESETVMStatus::EMULATION_INS_NUM_EXCEEDED;
			}
		}
	}
	return ESETVMStatus::SUCCESS;
}
std::optional<registerIntegerType> EVMExecutionUnit::readIntegerFromAddress(const registerIntegerType address, const MemoryAccessSize size)
{
	if (address >= m_memory.size())
	{
		return std::nullopt;
	}
	std::vector<uint8_t> tmp {m_memory.begin() + address, m_memory.begin() + address + static_cast<unsigned int>(size)};
	return utils::convertToInteger<registerIntegerType>(tmp);
}
std::optional<registerIntegerType> EVMExecutionUnit::getDataAccess(const DataAccess& da, const std::vector<registerIntegerType>& registers)
{
	registerIntegerType regVal {};
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
bool EVMExecutionUnit::saveDataAccess(registerIntegerType val, const DataAccess& da, std::vector<registerIntegerType>& registers, std::vector<uint8_t>& memory)
{
	registerIntegerType& regVal = registers.at(da.registerIndex);
	if (da.type == DataAccessType::REGISTER)
	{
		regVal = val;
		return true;
	}
	else if (da.type == DataAccessType::DEREFERENCE)
	{
		std::unique_lock l {writeMemoryMutex};
			
		size_t accessSize = static_cast<size_t>(da.accessSize);
		if (regVal > memory.size() - accessSize)
		{
			return false;
		}
		std::vector<uint8_t> data = utils::convertIntegerToBytes(val, accessSize);
		std::copy(data.cbegin(), data.cend(), memory.begin() + regVal);
		return true;
	}
	return false;
}
void EVMExecutionUnit::printCrashInfo (const EVMInstruction& instruction)
{
	std::unique_lock l {printCrashMutex};
	
	std::thread::id id = std::this_thread::get_id();
	std::cerr << "Program crashed <thread: " << id << "> " << "at instruction: ";
	const auto srcLine = m_disasm.getSourceCodeLineForIp(m_threadContext.ip);
	if (!srcLine.has_value())
	{
		std::cerr << "unknown" << std::endl;
	}
	std::cerr << srcLine.value() << std::endl;
	for (size_t regIter = 0; regIter <m_threadContext.registers.size(); regIter++)
	{
		std::cerr << "R" << regIter << "= " << std::hex << std::setfill('0') << std::setw(sizeof(registerIntegerType) * 2) << m_threadContext.registers.at(regIter) << std::endl << std::dec;
	}
}
bool EVMExecutionUnit::executeInstruction(const EVMInstruction& instruction)
{
	size_t nextIns = m_threadContext.ip + 1;
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
	m_threadContext.ip = nextIns;
	return true;
}
bool EVMExecutionUnit::mov(const EVMInstruction& instruction)
{
	const DataAccess& daArg1 = instruction.arguments.at(0).data.dataAccess;
	const DataAccess& daArg2 = instruction.arguments.at(1).data.dataAccess;
	
	const auto arg1Result = getDataAccess(daArg1, m_threadContext.registers);
	if (!arg1Result.has_value())
	{
		return false;
	}
	if (!saveDataAccess(arg1Result.value(), daArg2, m_threadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMExecutionUnit::loadConst(const EVMInstruction& instruction)
{
	const DataAccess& da = instruction.arguments.at(1).data.dataAccess;
	const registerIntegerType& val = instruction.arguments.at(0).data.constant;
	if (!saveDataAccess(val, da, m_threadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMExecutionUnit::performArithmeticOperation(const EVMInstruction& instruction)
{
	// use pointers to make it faster?
	std::vector<DataAccess> daArgs;
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	
	//if (daArgs.size() == 3)
	registerIntegerType arg0 {};
	const auto arg0Result = getDataAccess(daArgs.at(0), m_threadContext.registers);
	if (!arg0Result.has_value())
	{
		return false;
	}
	arg0 = arg0Result.value();
			
	registerIntegerType arg1 {};
	const auto arg1Result = getDataAccess(daArgs.at(1), m_threadContext.registers);
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
	return saveDataAccess(acc, daArgs.at(2), m_threadContext.registers, m_memory);
}
bool EVMExecutionUnit::compare (const EVMInstruction& instruction)
{
	std::vector<DataAccess> daArgs;
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	const auto arg1Result = getDataAccess(daArgs.at(0), m_threadContext.registers);
	const auto arg2Result = getDataAccess(daArgs.at(1), m_threadContext.registers);
	if ((!arg1Result.has_value()) || (!arg2Result.has_value()))
	{
		return false;
	}
	if (arg1Result.value() == arg2Result.value())
	{
		if (!saveDataAccess(0, daArgs.at(2), m_threadContext.registers, m_memory))
		{
			return false;
		}
	}
	else if (arg1Result.value() < arg2Result.value())
	{
		if (!saveDataAccess(-1, daArgs.at(2), m_threadContext.registers, m_memory))
		{
			return false;
		}
	}
	else if (arg1Result.value() > arg2Result.value())
	{
		if (!saveDataAccess(1, daArgs.at(2), m_threadContext.registers, m_memory))
		{
			return false;
		}
	}
	return true;
}
std::optional<size_t> EVMExecutionUnit::jump(const EVMInstruction& instruction)
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
std::optional<size_t> EVMExecutionUnit::jumpEqual(const EVMInstruction& instruction)
{
	DataAccess daArg1 = instruction.arguments.at(1).data.dataAccess;
	DataAccess daArg2 = instruction.arguments.at(2).data.dataAccess;
	const auto arg1Result = getDataAccess(daArg1, m_threadContext.registers);
	const auto arg2Result = getDataAccess(daArg2, m_threadContext.registers);
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
	return m_threadContext.ip + 1;
}
bool EVMExecutionUnit::read (const EVMInstruction& instruction)
{
	std::vector<DataAccess> daArgs {};
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	
	const auto arg1 = getDataAccess(daArgs.at(0), m_threadContext.registers); // offset in input file
	const auto arg2 = getDataAccess(daArgs.at(1), m_threadContext.registers); // number of bytes to read
	const auto arg3 = getDataAccess(daArgs.at(2), m_threadContext.registers); // memory address to which read bytes will be stored
	
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
	if (!saveDataAccess(inputFile.gcount(), daArgs.at(3), m_threadContext.registers, m_memory))
	{
		inputFile.close();
		return false;
	}
	inputFile.close();
	return true;
}
bool EVMExecutionUnit::write (const EVMInstruction& instruction)
{
	std::unique_lock l {writeFileMutex};
	
	std::vector<DataAccess> daArgs {};
	std::transform(instruction.arguments.cbegin(), instruction.arguments.cend(), std::back_inserter(daArgs), [](EVMArgument arg){ return arg.data.dataAccess; });
	const auto arg1 = getDataAccess(daArgs.at(0), m_threadContext.registers); // offset in output file
	const auto arg2 = getDataAccess(daArgs.at(1), m_threadContext.registers); // number of bytes to write
	const auto arg3 = getDataAccess(daArgs.at(2), m_threadContext.registers); // memory address from which bytes will be written
	
	if ((!arg1.has_value()) || (!arg2.has_value()) || (!arg3.has_value()))
	{
		return false;
	}
	if (m_binaryFilePath.size() == 0)
	{
		return false;
	}
	std::fstream outputFile {m_binaryFilePath, std::ios::binary | std::ios::out | std::ios::in};
	if (!outputFile.is_open())
	{
		outputFile.close();
		return false;
	}
	outputFile.seekp(arg1.value(), std::ios::beg); // should write zeros if beyond filesize
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
bool EVMExecutionUnit::consoleRead(const EVMInstruction& instruction)
{
	std::unique_lock l {consoleReadMutex};
	registerIntegerType val {};
	std::cin >> std::hex >> val;
	if (!saveDataAccess(val, instruction.arguments.at(0).data.dataAccess, m_threadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMExecutionUnit::consoleWrite(const EVMInstruction& instruction)
{
	std::unique_lock l (consoleWriteMutex);
	
	const DataAccess& da = instruction.arguments.at(0).data.dataAccess;
	const auto daResult = getDataAccess(da, m_threadContext.registers);
	if (!daResult.has_value())
	{
		return false;
	}
	registerIntegerType val = daResult.value();
	
	std::cout << std::hex << std::setfill('0') << std::setw(sizeof(val) * 2) << daResult.value() << std::endl << std::dec;
	return true;
}
bool EVMExecutionUnit::createThread(const EVMInstruction& instruction)
{
	uint32_t codeOffset = instruction.arguments.at(0).data.codeAddress;
	const auto insNum = m_disasm.insNumFromCodeOff(codeOffset);
	if (!insNum.has_value())
	{
		return false;
	}
	DataAccess da = instruction.arguments.at(1).data.dataAccess;
	
	std::promise<void> initPromise;
	std::future<void> initFuture = initPromise.get_future();

	std::thread t ([insNum, this, &initPromise]()
	{
		EVMContext newContext {m_threadContext};
		newContext.ip = insNum.value();
		EVMExecutionUnit executionUnit {m_instructions, m_memory, m_disasm, newContext, m_mutices, m_binaryFilePath, m_verbose, m_maxEmulatedInstructionCount, m_emulatedInstructionCount};
		initPromise.set_value();
		executionUnit.run();
	});
	initFuture.wait();
	registerIntegerType idHash = std::hash<std::thread::id>{}(t.get_id()); // not best idea but ID must be stored as integer
	m_threads.emplace(idHash, std::move(t));
	if (!saveDataAccess(idHash, da, m_threadContext.registers, m_memory))
	{
		return false;
	}
	return true;
}
bool EVMExecutionUnit::joinThread(const EVMInstruction& instruction)
{
	std::unique_lock l {joinMutex};
	DataAccess da = instruction.arguments.at(0).data.dataAccess;
	const auto threadId = getDataAccess(da, m_threadContext.registers);
	if (!threadId.has_value())
	{
		return false;
	}
	if (!m_threads.contains(threadId.value()))
	{
		return false;
	}
	std::thread& t = m_threads.at(threadId.value());
	if (!t.joinable())
	{
		return false;
	}
	t.join();
	return true;
}
bool EVMExecutionUnit::sleep(const EVMInstruction& instruction)
{
	const DataAccess& da = instruction.arguments.at(0).data.dataAccess;
	const auto sleepDuration = getDataAccess(da, m_threadContext.registers);
	if (!sleepDuration.has_value())
	{
		return false;
	}
	std::this_thread::sleep_for (std::chrono::milliseconds(sleepDuration.value()));
	return true;
}
std::optional<size_t> EVMExecutionUnit::call(const EVMInstruction &instruction)
{
	if (m_threadContext.ip + 1 > m_instructions.size())
	{
		return std::nullopt;
	}
	const auto jumpResult = jump(instruction);
	if (!jumpResult.has_value())
	{
		return std::nullopt;
	}
	m_threadContext.callStack.push(m_threadContext.ip + 1);
	return jumpResult.value();
}
std::optional<size_t> EVMExecutionUnit::ret(const EVMInstruction &instruction)
{
	if (m_threadContext.callStack.size() == 0)
	{
		return std::nullopt;
	}
	size_t retInsOff = m_threadContext.callStack.top();
	m_threadContext.callStack.pop();
	if (retInsOff >= m_instructions.size())
	{
		return std::nullopt;
	}
	return retInsOff;
}
bool EVMExecutionUnit::lock(const EVMInstruction &instruction)
{
	DataAccess da = instruction.arguments.at(0).data.dataAccess;
	const auto mutexObj = getDataAccess(da, m_threadContext.registers);
	if (!mutexObj.has_value())
	{
		return false;
	}

	if (m_mutices.contains(mutexObj.value()))
	{
		auto mutex = m_mutices.at(mutexObj.value());
		mutex->lock();
	}
	else
	{
		std::unique_lock l {muticesMutex};
		auto m = std::make_shared<std::mutex>();
		m->lock();
		m_mutices.emplace(mutexObj.value(), m);
	}
	return true;
}
bool EVMExecutionUnit::unlock(const EVMInstruction &instruction)
{
	std::unique_lock l {unlockMutex};
	DataAccess da = instruction.arguments.at(0).data.dataAccess;
	const auto mutexObj = getDataAccess(da, m_threadContext.registers);
	if (!mutexObj.has_value())
	{
		return false;
	}
	if (!m_mutices.contains(mutexObj.value()))
	{
		return false;
	}
	m_mutices.at(mutexObj.value())->unlock();
	return true;
}
