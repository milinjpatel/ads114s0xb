#pragma once
#include <cstdint>
#include <span>

namespace hal
{
    namespace spi
    {
        enum Result
        {
            SUCCESS
        };

        Result init();
        Result read(const std::span<uint8_t> readBuffer);
        Result write(const std::span<const uint8_t> writeBuffer);
    }
}