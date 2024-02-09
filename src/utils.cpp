#include "utils.h"

namespace utils
{
	std::streamsize getFileSize(std::ifstream& fileHandle)
	{
		fileHandle.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize length = fileHandle.gcount();
		fileHandle.clear();   //  Since ignore will have set eof.
		fileHandle.seekg(0, std::ios_base::beg);
		return length;
	}
	std::string convertToBitStream(const std::vector<std::byte>& bytes)
	{
		std::string bitStream;
		bitStream.reserve(bytes.size() * 8);
		for (const auto& it : bytes)
		{
			for (int bit = 7; bit >= 0; --bit)
			{
				bitStream.push_back((static_cast<uint8_t>(it) & (1 << bit)) ? '1' : '0');
			}
		}
		return bitStream;
	}
	std::string byteArrayToHexString(const std::vector<std::byte>& byteArray, uint32_t width)
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		for (size_t i = 0; i < byteArray.size(); ++i) 
		{
			ss << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(byteArray[i])) << " ";
			if ((i + 1) % width == 0)
			{
				ss << std::endl;
			}
		}
		return ss.str();
	}
}
