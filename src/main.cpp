#include "ESETVM.h"
#include "CLIArgParser.h"

int main (int argc, char** argv)
{
	CLIArgParser cliParser {argc, argv};
	if (cliParser.parseArguments())
	{
		cmdLineFlags CLIflags = cliParser.getFlags();
		ESETVM evm {cliParser.getInputPath(), cliParser.getOutpuPath()}; // outputPath empty if not set
		if (evm.init() != ESETVMStatus::SUCCESS)
		{
			return 1;
		}
		if (CLIflags.disassemble)
		{
			if (evm.disassemble() != ESETVMStatus::SUCCESS)
			{
				return 2;
			}
		}
		else if (CLIflags.run)
		{
			// to implement running .evm files
		}
	}
	return 0;
}
