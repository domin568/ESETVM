#pragma once
#include <fstream>
#include <limits>
#include <vector>
#include <sstream>
#include <iomanip>

static const int BITS_IN_BYTE = 8;

namespace utils
{
	std::streamsize getFileSize(std::ifstream& fileHandle);
	std::string convertToBitStream(const std::vector<std::byte>& bytes);
	std::string byteArrayToHexString(const std::vector<std::byte>& byteArray, uint32_t width);
}
