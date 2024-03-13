#pragma once
#include "utils.h"
#include <algorithm>
#include <bitset>
#include <inttypes.h>
#include <optional>
#include <string>
#include <vector>

enum class BitStreamReaderSeekStrategy
{
	BEG,
	CUR,
	END
};

class BitStreamReader
{
private:
	std::vector<bool> m_bitStream {};
	uint64_t m_bitStreamPosition {};
	
	template <typename T>
	std::optional<T> bitStreamToVar(std::vector<bool>::const_iterator begin, std::vector<bool>::const_iterator end, bool bigEndian = false)
	{
		if (std::distance(begin, end) > sizeof(T) * BITS_IN_BYTE)
		{
			return std::nullopt;
		}
		T var {};
		size_t bitIndex = 0;
		for (auto& it = begin; it != end; it++, bitIndex++)
		{
			if (*it == true)
			{
				if (bigEndian)
				{
					var |= static_cast<T>(1) << (end - it - 1);
				}
				else
				{
					var |= static_cast<T>(1) << bitIndex;
				}
			}
		}
		return var;
	}
	
public:
	BitStreamReader() = default;
	BitStreamReader(const std::vector<std::byte>& inputData);
	void init(const std::vector<std::byte>& inputData);
	bool seek(size_t offset, BitStreamReaderSeekStrategy strategy = BitStreamReaderSeekStrategy::CUR);
	std::optional<std::vector<bool>::const_iterator> readBits(size_t count, bool movePointer = true);
	uint64_t getStreamSize() const { return m_bitStream.size(); }
	uint64_t getStreamPosition() const { return m_bitStreamPosition; }
	
	template <typename T>
	std::optional<T> readVar(size_t overrideCountBytes = 0, bool movePointer = true, bool bigEndian = false)
	{
		std::vector<bool> bits;
		size_t bitsToRead = overrideCountBytes == 0 ? sizeof(T) * BITS_IN_BYTE : overrideCountBytes;

		if (const auto readBitsResult = readBits(bitsToRead, movePointer); readBitsResult.has_value())
		{
			auto endIt = readBitsResult.value();
			if (const auto varResult = bitStreamToVar<T>(endIt - bitsToRead, endIt, bigEndian); varResult.has_value())
			{
				return varResult.value();
			}
		}
		
		return std::nullopt;
	}
	
};
