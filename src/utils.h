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
	std::string convertToBitStream(std::vector<uint8_t>& bytes);
	std::string byteArrayToHexString(std::vector<uint8_t> byteArray, uint32_t width);

	template <typename T>
	bool bitStreamToVar(std::string bitStream, T& var)
	{
		var = 0;
		for (int bitIndex = 0; bitIndex < bitStream.length(); bitIndex++)
		{
			if (bitStream[bitIndex] == '1')
			{
				var |= static_cast<T>(1) << bitIndex;
			}
		}
		return true;
	}
}