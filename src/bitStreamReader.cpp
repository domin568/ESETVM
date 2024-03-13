#include "BitStreamReader.h"

BitStreamReader::BitStreamReader(const std::vector<std::byte>& inputData)
{
	init(inputData);
}
void BitStreamReader::init(const std::vector<std::byte>& inputData)
{
	m_bitStream.reserve(inputData.size() * BITS_IN_BYTE);
	for (const auto& byte : inputData)
	{
		for (int i = BITS_IN_BYTE - 1; i >= 0; --i)
		{
			bool bit = static_cast<uint8_t>(byte) & (1 << i);
			m_bitStream.push_back(bit);
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
std::optional<std::vector<bool>::const_iterator> BitStreamReader::readBits(size_t count, bool movePointer)
{
	if (m_bitStreamPosition + count > m_bitStream.size())
	{
		return std::nullopt;
	}
	const auto begin = m_bitStream.cbegin() + m_bitStreamPosition;
	const auto end = begin + count;
	if (movePointer)
	{
		m_bitStreamPosition += count;
	}
	return end;
}

