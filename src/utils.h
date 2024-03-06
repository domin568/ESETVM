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

	template <typename T>
	T convertToInteger(const std::vector<uint8_t>& bytes)
	{
		static_assert(sizeof(T) * 8 >= 8, "Integer type must be at least 8 bits");

		T result = 0;
		size_t size = std::min(sizeof(T), bytes.size());

		for (size_t i = 0; i < size; i++)
		{
			result |= static_cast<T>(bytes[i]) << (8 * i);
		}
		return result;
	}
	template <typename T>
	std::vector<uint8_t> convertIntegerToBytes (const T& integer, const size_t size = sizeof(T))
	{
		std::vector<uint8_t> result {};
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&integer);
		result.insert(result.begin(), bytes, bytes + size);
		return result;
	}
}
