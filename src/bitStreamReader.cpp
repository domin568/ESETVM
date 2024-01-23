#include "bitStreamReader.h"

bitStreamReader::bitStreamReader(std::string bitStream) :
	m_bitStream(bitStream),
	m_bitStreamPosition(0)
{}
std::string bitStreamReader::readBits(uint64_t count, bool movePointer)
{
	if (m_bitStreamPosition + count > m_bitStream.size())
	{
		return "";
	}
	std::string bits = m_bitStream.substr(m_bitStreamPosition, count);
	if (movePointer)
	{
		m_bitStreamPosition += count;
	}
	return bits;
}
bool bitStreamReader::seek(size_t offset, BitStreamReaderSeekStrategy strategy)
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
