#include "CLIArgParser.h"
#include "ESETVM.h"

int main (int argc, char** argv)
{
	CLIArgParser cliParser {argc, (const char**) argv};
	if (!cliParser.parseArguments())
	{
		return static_cast<int>(ESETVMStatus::CLI_ARG_PARSING_ERROR);
	}
	cmdLineFlags cliFlags = cliParser.getFlags();
	ESETVM evm {cliParser.getInputPath(), cliParser.getOutputPath(), cliFlags.verbose}; // outputPath empty if not set
	ESETVMStatus initStatus = evm.init();
	if (initStatus != ESETVMStatus::SUCCESS)
	{
		return static_cast<int>(initStatus);
	}
	if (cliFlags.disassemble)
	{
		ESETVMStatus saveSrcStatus = evm.saveSourceCode();
		if (saveSrcStatus != ESETVMStatus::SUCCESS)
		{
			return static_cast<int>(saveSrcStatus);
		}
	}
	else if (cliFlags.run)
	{
		ESETVMStatus runStatus = evm.run(cliParser.getBinaryFilePath());
		if (runStatus != ESETVMStatus::SUCCESS)
		{
			return static_cast<int>(runStatus);
		}
	}
	return static_cast<int>(ESETVMStatus::SUCCESS);
}
