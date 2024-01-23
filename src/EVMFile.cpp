#include "EVMFile.h"

EVMFile::EVMFile(std::string filePath):
	m_fileSize(0),
	m_error(EVMFileStatus::SUCCESS)
{
	m_fileHandle.open(filePath, std::ios::binary);
	if (!m_fileHandle.is_open()) 
	{
		m_error = EVMFileStatus::FILE_OPEN_ERROR;
		return;
	}
	m_fileSize = utils::getFileSize(m_fileHandle);
	if (m_fileSize > Max_Input_File_Size)
	{
		m_error = EVMFileStatus::FILE_TOO_BIG;
		return;
	}
	if (!parseFile())
	{
		return;
	}
}
bool EVMFile::parseFile()
{
	// parse header
	m_fileHandle.read(reinterpret_cast<char*>(&m_header), sizeof(m_header));
	if (m_fileHandle.fail()) 
	{
		m_error = EVMFileStatus::FILE_READ_ERROR;
		return false;
	}
	if (memcmp(m_header.magic, EVM_Magic, sizeof(m_header.magic)))
	{
		m_error = EVMFileStatus::NOT_EVM_FILE;
		return false;
	}
	if (m_header.dataSize < m_header.initialDataSize || m_header.codeSize + m_header.initialDataSize + sizeof(m_header) != m_fileSize)
	{
		m_error = EVMFileStatus::FILE_CORRUPTED;
		return false;
	}

	// read code & data bytes
	m_codeBytes.reserve(m_header.codeSize);
	m_dataBytes.reserve(m_header.initialDataSize);

	std::istreambuf_iterator<char> fileIterator(m_fileHandle);
	std::copy_n(fileIterator, m_header.codeSize, std::back_inserter(m_codeBytes));
	if (m_fileHandle.fail())
	{
		m_error = EVMFileStatus::FILE_READ_ERROR;
		return false;
	}
	if (m_header.initialDataSize > 0)
	{
		if (!m_fileHandle.seekg(1, std::ios::cur))
		{
			return false;
		}
		std::copy_n(fileIterator, m_header.initialDataSize, std::back_inserter(m_dataBytes));
		if (m_fileHandle.fail())
		{
			m_error = EVMFileStatus::FILE_READ_ERROR;
			return false;
		}
	}
	return true;
}
