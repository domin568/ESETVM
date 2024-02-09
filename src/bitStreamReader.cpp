#include "BitStreamReader.h"

BitStreamReader::BitStreamReader(const std::vector<std::byte>& inputData)
{
	init(inputData);
}
void BitStreamReader::init(const std::vector<std::byte>& inputData)
{
	for (const auto& byte : inputData)
	{
		std::bitset<BITS_IN_BYTE> bits(static_cast<unsigned char>(byte));
		for (int i = 7; i >= 0; --i)
		{
			m_bitStream.push_back(bits[i]);
		}
	}
}

bool BitStreamReader::seek(size_t offset, BitStreamReaderSeekStrategy strategy)
{
	if (strategy == BitStreamReaderSeekStrategy::CUR)
	{
		if (m_bitStreamPosition + offset > m_bitStream.size())
		{
			return false;
		}
		m_bitStreamPosition += offset;
	}
	if (offset >= m_bitStream.size())
	{
		return false;
	}
	else if (strategy == BitStreamReaderSeekStrategy::BEG)
	{
		m_bitStreamPosition = offset;
	}
	else if (strategy == BitStreamReaderSeekStrategy::END)
	{
		m_bitStreamPosition = m_bitStream.size() - offset - 1;
	}
	return true;
}
std::optional<std::vector<bool>> BitStreamReader::readBits(size_t count, bool movePointer)
{
	if (m_bitStreamPosition + count > m_bitStream.size())
	{
		return std::nullopt;
	}
	std::vector<bool> bits {m_bitStream.cbegin() + m_bitStreamPosition, m_bitStream.cbegin() + m_bitStreamPosition + count};
	if (movePointer)
	{
		m_bitStreamPosition += count;
	}
	return bits;
}

