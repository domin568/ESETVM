#pragma once

#include "EVMTypes.h"
#include "utils.h"
#include <cstring>
#include <fstream>
#include <inttypes.h>
#include <vector>

class EVMFile
{
private:
	static constexpr char EVM_Magic[] = "ESET-VM2";
	static constexpr long long Max_Input_File_Size = 256ULL * 1024ULL * 1024ULL; // 256 MB

#pragma pack(1)
	struct EVMHeader
	{
		char magic[8];
		uint32_t codeSize;
		uint32_t dataSize;
		uint32_t initialDataSize;
	};
#pragma pack()

	std::ifstream m_fileHandle {};
	size_t m_fileSize {};
	EVMHeader m_header{};
	ESETVMStatus m_error {ESETVMStatus::SUCCESS};
	std::vector<std::byte> m_codeBytes {};
	std::vector<std::byte> m_dataBytes {};

	bool parseFile();
public:
	EVMFile() = default;
	EVMFile(std::string filePath);
	void init(std::string filePath);

	ESETVMStatus getError() const { return m_error; }
	std::vector<std::byte> getCodeBytes() const { return m_codeBytes; }
	std::vector<std::byte> getDataBytes() const { return m_dataBytes; }
	uint32_t getInitialDataSize() const { return m_header.initialDataSize; }
	uint32_t getcodeSize() const{ return m_header.codeSize; }
	uint32_t getDataSize() const{ return m_header.dataSize; }
};

