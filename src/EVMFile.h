#pragma once
#include "utils.h"
#include <inttypes.h>
#include <fstream>
#include <vector>

enum class EVMFileStatus
{
	SUCCESS,
	FILE_OPEN_ERROR,
	FILE_READ_ERROR,
	NOT_EVM_FILE,
	FILE_CORRUPTED,
	FILE_TOO_BIG
};

class EVMFile
{
private:
	static constexpr char EVM_Magic[] = "ESET-VM2";
	static constexpr long long Max_Input_File_Size = 256LLU * 1024ULL * 1024ULL; // 256 MB

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
	std::streamsize m_fileSize {};
	EVMHeader m_header{};
	EVMFileStatus m_error {EVMFileStatus::SUCCESS};
	std::vector<std::byte> m_codeBytes {};
	std::vector<std::byte> m_dataBytes {};

	bool parseFile();
public:
	EVMFile(){};
	EVMFile(std::string filePath);
	void init(std::string filePath);

	EVMFileStatus getError() const { return m_error; }
	std::vector<std::byte> getCodeBytes() const { return m_codeBytes; }
	std::vector<std::byte> getDataBytes() const { return m_dataBytes; }
	uint32_t getInitialDataSize() const { return m_header.initialDataSize; }
	uint32_t getcodeSize() const{ return m_header.codeSize; }
	uint32_t getDataSize() const{ return m_header.dataSize; }
};

