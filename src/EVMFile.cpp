#include "EVMFile.h"

EVMFile::EVMFile(std::string filePath)
{
	init(filePath);
}
void EVMFile::init(std::string filePath)
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

	m_codeBytes.resize(m_header.codeSize);
	m_dataBytes.resize(m_header.initialDataSize);

	m_fileHandle.read(reinterpret_cast<char*>(m_codeBytes.data()), m_header.codeSize);
	if (m_fileHandle.fail())
	{
		m_error = EVMFileStatus::FILE_READ_ERROR;
		return false;
	}
	if (m_header.initialDataSize > 0)
	{
		m_fileHandle.read(reinterpret_cast<char*>(m_dataBytes.data()), m_header.initialDataSize);
		if (m_fileHandle.fail())
		{
			m_error = EVMFileStatus::FILE_READ_ERROR;
			return false;
		}
	}
	return true;
}
