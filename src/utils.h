#pragma once
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

static const int BITS_IN_BYTE = 8;

namespace utils
{
	std::streamsize getFileSize(std::ifstream& fileHandle);
	std::string byteArrayToHexString(const std::vector<std::byte>& byteArray, uint32_t width);
}
