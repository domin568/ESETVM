#include "EVMDisasm.h"
#include "EVMFile.h"
#include <iostream>

int main(int argc, char ** argv)
{
    if (argc < 3)
    {
        std::cout << "EVM disasm utility, please provide <input.evm> <output.easm>" << std::endl;
        return 1;
    }
    std::string inputFilePath (argv[1]);
    EVMFile file(inputFilePath);
    if (file.getError() != EVMFileStatus::SUCCESS)
    {
        std::cerr << "Input file error" << std::endl;
        return 2;
    }
    std::vector<uint8_t> codeBytes = file.getCodeBytes();
    std::string bitStream = utils::convertToBitStream(codeBytes);
    EVMDisasm disasm(bitStream);
	std::vector<EVMInstruction> instructions;
    if (!disasm.parseInstructions(instructions))
    {
        std::cerr << "Instruction parsing error" << std::endl;
        return 3;
    }
	std::vector<std::string> sourceCodeLines{};
    if (!disasm.convertInstructionsToSourceCode(instructions, sourceCodeLines))
    {
        std::cerr << "Source code produce error" << std::endl;
        return 4;
    }
    
    std::ofstream outputFile(argv[2]);
    outputFile << ".dataSize " << file.getDataSize() << std::endl;
    if (file.getInitialDataSize() > 0)
    {
        outputFile << ".data" << std::endl << std::endl;
		std::vector<uint8_t> dataBytes = file.getDataBytes();
		outputFile << utils::byteArrayToHexString(dataBytes, 40);
		outputFile << std::endl << std::endl;
    }
    outputFile << ".code" << std::endl << std::endl;
    for (const auto& line : sourceCodeLines)
    {
        outputFile << line << std::endl;
    }
    outputFile.close();
    std::cout << "Success" << std::endl;
}
