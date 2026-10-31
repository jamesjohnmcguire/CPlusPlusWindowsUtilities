#include "Utf8.h"

bool HasUtf8Bom(const std::uint8_t* data, std::size_t length) noexcept
{
	return data != nullptr && length >= 3 &&
		data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF;
}
