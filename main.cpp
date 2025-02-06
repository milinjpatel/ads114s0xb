#include <cstdio>

#include "ads114s0xb/ads114s0xb.hpp"
#include "hal/spi.hpp"

int main()
{
    if (ads114s0xb::init() != 0)
    {
        return -1;
    }

    // Example of how to read a register.
    uint8_t devId;
    if (ads114s0xb::readRegister(ads114s0xb::RegisterAddresses::ID, devId) != 0)
    {
        return -1;
    }
    printf("Device ID: %d\n", devId);

    if (ads114s0xb::startConversions() != 0)
    {
        return -1;
    }

    // For the purposes of this, we just loop once to read ADC values from all channels. This serves
    // as a basic example on how to change which ADC channel is active and how to read a value from
    // the ADC. In a real system, we would continuously read data and use it for something. We would
    // also have a more event-driven interrupt based system based on nDRDY toggling.
    for (uint8_t channel = 0; channel < 12; channel++)
    {
        if (ads114s0xb::changeActiveChannel(channel) != 0)
        {
            return -1;
        }

        float adcVoltage;
        if (ads114s0xb::readAdcValue(adcVoltage) != 0)
        {
            return -1;
        }

        printf("Read ADC Voltage: %f\n", adcVoltage);
    }

    if (ads114s0xb::stopConversions() != 0)
    {
        return -1;
    }

    return 0;
}
