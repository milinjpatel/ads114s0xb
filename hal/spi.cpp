#include <string.h>

#include "../ads114s0xb/ads114s0xb.hpp"
#include "../hal/spi.hpp"

// This file is a placeholder for the HAL SPI API. The code implementation for this is dependent
// on the microcontroller/processor that we're running on. Typically, I would use the HAL generation
// code provided by the microcontroller vendor as a starting point. Once I have something up and
// running, I would benchmark and optimize as needed. In the most extreme case, I might have to
// implement my own HAL SPI that's highly optimized for our system. Since I don't know the hardware
// we're using, I'm implementing a mock layer that can be used for testing, which would typically be
// part of unit tests through something like googletest.

const std::array<uint16_t, 12> TEST_DATA{0x7FFF, 0x5AC2, 0x3049, 0x1A,   0x1,    0x0,
                                         0xFFFF, 0xF148, 0xA4F2, 0x9287, 0x8601, 0x8000};

constexpr uint8_t RDATA = 0x12;
constexpr uint8_t RREG = 32;
constexpr uint8_t WREG = 64;
constexpr uint8_t DEFAULT_TEST_DEV_ID = 4;
constexpr uint8_t INPMUX_MUXP_SHIFT = 4;

constexpr uint8_t MAX_READ_DATA_SIZE = 5;

static std::array<uint8_t, MAX_READ_DATA_SIZE> readData{};
static uint8_t activeChannel = 0;

namespace hal
{
    namespace spi
    {
        Result init() { return Result::SUCCESS; }

        Result read(const std::span<uint8_t> readBuffer)
        {
            if (readData[0] == RDATA)
            {
                readBuffer[0] = TEST_DATA[activeChannel] & 0xFF;
                readBuffer[1] = TEST_DATA[activeChannel] >> 8;
            }
            else if ((readData[0] & 0xE0) == RREG)
            {
                const uint8_t regAddr = readData[0] & 0x1F;
                if (regAddr == ads114s0xb::RegisterAddresses::ID)
                {
                    readBuffer[0] = DEFAULT_TEST_DEV_ID;
                }
                else if (regAddr == ads114s0xb::RegisterAddresses::STATUS)
                {
                    readBuffer[0] = 0;
                }
            }

            return Result::SUCCESS;
        }

        Result write(const std::span<const uint8_t> writeBuffer)
        {
            if ((readData[0] & 0xE0) == WREG)
            {
                const uint8_t regAddr = readData[0] & 0x1F;
                if (regAddr == ads114s0xb::RegisterAddresses::INPMUX)
                {
                    activeChannel = readData[2] >> INPMUX_MUXP_SHIFT;
                }
            }
            memcpy(readData.data(), writeBuffer.data(), writeBuffer.size());
            return Result::SUCCESS;
        }
    }
}