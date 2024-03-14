#include "CLIArgParser.h"

CLIArgParser::CLIArgParser(int argc, const char** argv)
{
	std::copy(argv, argv+argc, std::back_inserter(m_args)); // assume argv is always provided by runtime
}
void CLIArgParser::showHelp()
{
	std::cout << "Usage: esetvm [-h] [-v] [-d] [-r] <input.evm> <output.easm> [-b] <file.bin>" << std::endl;
	std::cout << "-h shows this help" << std::endl;
	std::cout << "-v enables verbose mode" << std::endl;
	std::cout << "-d <input.evm> <output.easm> deassembles input file and saves it to output" << std::endl;
	std::cout << "-r <input.evm> runs .evm file" << std::endl;
	std::cout << "-b <file.bin> passes file to program" << std::endl;
}
bool CLIArgParser::parseArguments ()
{
	if (m_args.size() < 2)
	{
		showHelp();
		return false;
	}
	for (const auto& argBind : m_cmdLineArgBinds)
	{
		const auto& checkedArg = std::find(m_args.cbegin(), m_args.cend(), argBind.first);
		if (checkedArg != m_args.cend())
		{
			*(argBind.second) = true;
			if (*checkedArg == "-r" && checkedArg + 1 != m_args.cend())
			{
				m_inputPath = *(checkedArg + 1);
			}
			else if (*checkedArg == "-d" && checkedArg + 1 != m_args.cend() && checkedArg + 2 != m_args.cend())
			{
				m_inputPath = *(checkedArg + 1);
				m_outputPath = *(checkedArg + 2);
			}
			else if (*checkedArg == "-b" && checkedArg + 1 != m_args.cend())
			{
				m_binaryFilePath = *(checkedArg + 1);
			}
		}
	}
		
	if (!m_inputPath.empty() && !std::filesystem::exists(m_inputPath))
	{
		std::cerr << "Invalid input file" << std::endl;
		return false;
	}
	else if (m_cliFlags.disassemble && (m_inputPath.empty() || m_outputPath.empty()))
	{
		std::cerr << "If you want to dissasemble .evm file please provide <file.evm> <output.easm>" << std::endl;
		return false;
	}
	else if (m_cliFlags.run && m_inputPath.empty())
	{
		std::cerr << "If you want to run evm program please provide -i <file.evm>" << std::endl;
		return false;
	}
	else if ( (m_cliFlags.disassemble && (m_cliFlags.run || m_cliFlags.binaryFile)) || (m_cliFlags.binaryFile && !(m_cliFlags.disassemble) && !(m_cliFlags.run)))
	{
		std::cout << "Invalid arguments" << std::endl;
		return false;
	}
	else if (!m_binaryFilePath.empty() && !std::filesystem::exists(m_binaryFilePath))
	{
		std::cerr << "Invalid binary file" << std::endl;
		return false;
	}
	return true;
}
