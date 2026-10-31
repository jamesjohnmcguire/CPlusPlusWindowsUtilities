#include "Utf8.h"

#include <algorithm>
#include <array>

namespace
{
	// The UTF-8 byte order mark is the three-byte sequence EF BB BF.
	constexpr std::array<std::uint8_t, 3> Utf8BomBytes =
		{0xEF, 0xBB, 0xBF};
}

bool HasUtf8Bom(const std::uint8_t* data, std::size_t length) noexcept
{
	bool hasUtf8Bom = false;

	if (data != nullptr)
	{
		constexpr std::size_t bomSize = Utf8BomBytes.size();

		if (bomSize > length)
		{
			hasUtf8Bom =
				std::equal(Utf8BomBytes.begin(), Utf8BomBytes.end(), data);
		}
	}

	return hasUtf8Bom;
}
