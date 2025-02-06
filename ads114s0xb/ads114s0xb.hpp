#pragma once
#include <cstdint>

namespace ads114s0xb
{
    enum RegisterAddresses : uint8_t
    {
        ID,
        STATUS,
        INPMUX,
        PGA,
        DATARATE,
        REF,
        IDACMAG,
        IDACMUX,
        VBIAS,
        SYS,
        RESERVED_0,
        OFCAL0,
        OFCAL1,
        RESERVED_1,
        FSCAL0,
        FSCAL1,
        GPIODAT,
        GPIOCON
    };

    // Using int8_t as the return type here but can also create an enum to capture all possible
    // errors, if needed.
    int8_t init();
    int8_t readRegister(const RegisterAddresses regAddr, uint8_t& regValue);
    int8_t readAdcValue(float& voltage);
    int8_t changeActiveChannel(const uint8_t channel);
    int8_t startConversions();
    int8_t stopConversions();
}