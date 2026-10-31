#pragma once

#include <cstddef>
#include <cstdint>

bool HasUtf8Bom(const std::uint8_t* data, std::size_t length) noexcept;
