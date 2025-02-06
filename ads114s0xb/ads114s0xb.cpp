#include <chrono>
#include <thread>

#include "../ads114s0xb/ads114s0xb.hpp"
#include "../hal/spi.hpp"

namespace
{
    enum Commands : uint8_t
    {
        NOP = 0x0,
        WAKEUP = 0x2,
        POWERDOWN = 0x4,
        RESET = 0x6,
        START = 0x8,
        STOP = 0xA,
        SYOCAL = 0x16,
        SYGCAL = 0x17,
        SFOCAL = 0x19,
        RDATA = 0x12,
        RREG = 0b0010'0000,
        WREG = 0b0100'0000
    };

    constexpr std::chrono::microseconds POWER_ON_RESET_WAIT_TIME = std::chrono::microseconds(2200);

    constexpr uint8_t STATUS_nRDY_MASK = 0b100'0000;
    constexpr uint8_t ID_DEV_ID_MASK = 0b111;

    constexpr uint8_t INPMUX_AIN0 = 0b0;
    constexpr uint8_t INPMUX_AINCOM = 0b1100;
    constexpr uint8_t INPMUX_MUXP_SHIFT = 4;
    constexpr uint8_t INPMUX_VALUE = (INPMUX_AIN0 << INPMUX_MUXP_SHIFT) | INPMUX_AINCOM;

    constexpr uint8_t PGA_EN = 0b01;
    constexpr uint8_t PGA_SHIFT = 3;
    constexpr uint8_t PGA_VALUE = PGA_EN << PGA_SHIFT;

    constexpr uint8_t REF_REFCON_INTERNAL_EN = 0b01;
    constexpr uint8_t REF_REFSEL_SHIFT = 2;
    constexpr uint8_t REF_REFSEL_INTERNAL = 0b10;
    constexpr uint8_t REF_REFP_SHIFT = 5;
    constexpr uint8_t REF_REFP_BYPASS = 0b1;
    constexpr uint8_t REF_VALUE = (REF_REFP_BYPASS << REF_REFP_SHIFT) |
                                  (REF_REFSEL_INTERNAL << REF_REFSEL_SHIFT) |
                                  REF_REFCON_INTERNAL_EN;

    // Assuming we're using the 12 channel part for the DEV_ID.
    constexpr uint8_t EXPECTED_DEV_ID = 0b100;
    constexpr uint8_t MAX_NUM_CHANNELS = 12;

    // We could calculate these based on the default register settings we have defined so that we
    // don't have to update two variables if these ever change.
    constexpr float VREF = 2.5;
    constexpr float GAIN = 1;
    constexpr float LSB = (2 * VREF / GAIN) / (1 << 16);
}

static bool initialized;
static bool conversionsEnabled = false;

namespace ads114s0xb
{
    int8_t writeRegister(const RegisterAddresses regAddr, const uint8_t regValue);

    int8_t init()
    {
        initialized = false;

        std::this_thread::sleep_for(POWER_ON_RESET_WAIT_TIME);

        uint8_t regValue;
        if ((readRegister(RegisterAddresses::STATUS, regValue) != 0) ||
            ((regValue & STATUS_nRDY_MASK) != 0))
        {
            return -1;
        }

        if ((readRegister(RegisterAddresses::ID, regValue) != 0) ||
            ((regValue & ID_DEV_ID_MASK) != EXPECTED_DEV_ID))
        {
            return -1;
        }

        // Skipping the calibration commands. In a more complete implementation and with more time,
        // we could investigate which calibration commands we should run for our system.

        // Setting the desired register settings here. These will vary based on the hardware design.
        // For this purpose, we're enabling the PGA, setting up a single-ended input, and enabling
        // the internal reference.
        if (writeRegister(RegisterAddresses::INPMUX, INPMUX_VALUE) != 0)
        {
            return -1;
        }

        if (writeRegister(RegisterAddresses::PGA, PGA_VALUE) != 0)
        {
            return -1;
        }

        if (writeRegister(RegisterAddresses::REF, REF_VALUE) != 0)
        {
            return -1;
        }

        initialized = true;
        return 0;
    }

    int8_t readRegister(const RegisterAddresses regAddr, uint8_t& regValue)
    {
        const std::array<uint8_t, 2> readTransaction = {RREG | regAddr, 0};
        if (hal::spi::write(readTransaction) != hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        if (hal::spi::read({&regValue, sizeof(regValue)}) != hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        return 0;
    }

    int8_t writeRegister(const RegisterAddresses regAddr, const uint8_t regValue)
    {
        const std::array<uint8_t, 3> writeTransaction = {WREG | regAddr, 0, regValue};
        if (hal::spi::write(writeTransaction) != hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        return 0;
    }

    float adcCodeToVoltage(const int16_t adcCode) { return adcCode * LSB; }

    int8_t readAdcValue(float& voltage)
    {
        if (!initialized || !conversionsEnabled)
        {
            return -1;
        }

        // In a real system, we would have a GPIO that is connected to nDRDY that
        // we would monitor to know when data is ready from the ADC. We could either poll the GPIO
        // or setup an interrupt for an event-driven system.

        const uint8_t readDataTransaction = RDATA;
        if (hal::spi::write({&readDataTransaction, sizeof(readDataTransaction)}) !=
            hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        std::array<uint8_t, 2> readData;
        if (hal::spi::read(readData) != hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        const int16_t adcCode = (readData[1] << 8) | readData[0];
        voltage = adcCodeToVoltage(adcCode);

        return 0;
    }

    int8_t changeActiveChannel(const uint8_t channel)
    {
        if (!initialized || (channel >= MAX_NUM_CHANNELS))
        {
            return -1;
        }

        if (writeRegister(RegisterAddresses::INPMUX, channel << INPMUX_MUXP_SHIFT) != 0)
        {
            return -1;
        }

        return 0;
    }

    int8_t startConversions()
    {
        const uint8_t startTransaction = START;
        if (hal::spi::write({&startTransaction, sizeof(startTransaction)}) !=
            hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        conversionsEnabled = true;

        return 0;
    }

    int8_t stopConversions()
    {
        const uint8_t stopTransaction = STOP;
        if (hal::spi::write({&stopTransaction, sizeof(stopTransaction)}) !=
            hal::spi::Result::SUCCESS)
        {
            return -1;
        }

        conversionsEnabled = false;

        return 0;
    }
}