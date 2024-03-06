#pragma once
#include "utils.h"
#include <algorithm>
#include <bitset>
#include <inttypes.h>
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
	std::optional<T> bitStreamToVar(const std::vector<bool>& bitStream, bool bigEndian = false)
	{
		if (bitStream.size() > sizeof(T) * BITS_IN_BYTE)
		{
			return std::nullopt;
		}
		T var {};
		for (size_t bitIndex = 0; bitIndex < bitStream.size(); bitIndex++)
		{
			if (bitStream.at(bitIndex) == true)
			{
				if (bigEndian)
				{
					var |= static_cast<T>(1) << (bitStream.size() - bitIndex - 1);
				}
				else
				{
					var |= static_cast<T>(1) << (bitIndex);
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
	std::optional<std::vector<bool>> readBits(size_t count, bool movePointer = true);
	uint64_t getStreamSize() { return m_bitStream.size(); }
	uint64_t getStreamPosition() const { return m_bitStreamPosition; }
	
	template <typename T>
	std::optional<T> readVar(size_t overrideCountBytes = 0, bool movePointer = true, bool bigEndian = false)
	{
		std::vector<bool> bits;
		if (overrideCountBytes == 0)
		{
			if (const auto readBitsResult = readBits(sizeof(T) * BITS_IN_BYTE, movePointer); readBitsResult.has_value())
			{
				bits = readBitsResult.value();
			}
		}
		else
		{
			if (const auto readBitsResult = readBits(overrideCountBytes, movePointer); readBitsResult.has_value())
			{
				bits = readBitsResult.value();
			}
		}
		if (const auto varResult = bitStreamToVar<T>(bits, bigEndian); varResult.has_value())
		{
			return varResult.value();
		}
		return std::nullopt;
	}
	
};
