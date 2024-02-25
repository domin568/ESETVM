#include "ESETVM.h"
#include "CLIArgParser.h"

int main (int argc, char** argv)
{
	CLIArgParser cliParser {argc, (const char**) argv};
	if (cliParser.parseArguments())
	{
		cmdLineFlags cliFlags = cliParser.getFlags();
		ESETVM evm {cliParser.getInputPath(), cliParser.getOutpuPath(), cliFlags.verbose}; // outputPath empty if not set
		if (evm.init() != ESETVMStatus::SUCCESS)
		{
			return 1;
		}
		if (cliFlags.disassemble)
		{
			if (evm.saveSourceCode() != ESETVMStatus::SUCCESS)
			{
				return 2;
			}
		}
		else if (cliFlags.run)
		{
			if (evm.run(cliParser.getBinaryFilePath()) != ESETVMStatus::SUCCESS)
			{
				return 3;
			}
		}
	}
	return 0;
}
