#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>

struct cmdLineFlags
{
	bool help;
	bool verbose;
	bool disassemble;
	bool run;
	bool binaryFile;
};

class CLIArgParser
{
private:
	cmdLineFlags m_cliFlags {};
	std::unordered_map<std::string, bool*> m_cmdLineArgBinds =
	{
		{"-h", &m_cliFlags.help},
		{"-v", &m_cliFlags.verbose},
		{"-d", &m_cliFlags.disassemble},
		{"-r", &m_cliFlags.run},
		{"-b", &m_cliFlags.binaryFile}
	};
	std::vector<std::string> m_args {};
	std::string m_inputPath {};
	std::string m_outputPath {};
	std::string m_binaryFilePath {};
public:

	CLIArgParser(int argc, const char** argv);
	bool parseArguments ();
	void showHelp();
	cmdLineFlags getFlags() const { return m_cliFlags; }
	std::string getInputPath() const { return m_inputPath; }
	std::string getOutputPath() const { return m_outputPath; }
	std::string getBinaryFilePath() const { return m_binaryFilePath; }
};
