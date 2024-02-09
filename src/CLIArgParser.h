#pragma once

#include <string>
#include <algorithm>
#include <iostream>
#include <filesystem>

struct cmdLineFlags
{
	bool help;
	bool verbose;
	bool disassemble;
	bool run;
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
		{"-r", &m_cliFlags.run}
	};
	std::vector<std::string> m_args {};
	std::string m_inputPath {};
	std::string m_outputPath {};
public:

	CLIArgParser(int argc, char** argv);
	bool parseArguments ();
	void showHelp();
	cmdLineFlags getFlags() const { return m_cliFlags; }
	std::string getInputPath() const { return m_inputPath; }
	std::string getOutpuPath() const { return m_outputPath; }
};
