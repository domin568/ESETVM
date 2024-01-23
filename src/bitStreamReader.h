#pragma once
#include "utils.h"
#include <inttypes.h>
#include <string>

enum BitStreamReaderSeekStrategy
{
	BEG,
	CUR,
	END
};

class bitStreamReader
{
private:
	std::string m_bitStream;
	uint64_t m_bitStreamPosition;
public:
	bitStreamReader(std::string bitStream);
	std::string readBits(uint64_t count, bool movePointer = true);
	uint64_t getStreamSize() { return m_bitStream.length(); }
	bool seek(size_t offset, BitStreamReaderSeekStrategy strategy = BitStreamReaderSeekStrategy::CUR);
	uint64_t getStreamPosition() const { return m_bitStreamPosition; }
	template <typename T>
	bool readVar(T& var, int overrideCountBytes = 0)
	{
		std::string bits{};
		if (overrideCountBytes == 0)
		{
			bits = readBits(sizeof(var) * BITS_IN_BYTE);
		}
		else
		{
			bits = readBits(overrideCountBytes);
		}
		if (!utils::bitStreamToVar(bits, var))
		{
			return false;
		}
		return true;
	}
};