# ADS114S0xB

A basic driver for the ADS114S0xB. The driver is capable of reading and writing all registers and reading/converting codes from the ADC. This driver does not implement any HAL functionality as it is a generic driver that can be adapted to different hardware platforms. Instead, the HAL is replaced with a mock layer that allows for testing.

## Building Software

Required Software:
1. Python >= 3.8.0
2. scons
3. gcc12 and g++12 or newer

Run 'scons' in the root directory of this file. A binary will be placed in the root directory.

## Usage

The `ads114s0xb` namespace provides the API to interact with the ADS114S0xB. Functions are provided to initialize the device, read registers, start and stop conversions, and get ADC values that are already converted from ADC codes.

The `hal::spi` layer is currently implemented as a mock layer and must be implemented for your hardware platform.

The `main` function provides a basic example program of the API in action.

## Future Improvements

1. Improve the build scripts to place object files and final binary into a separate folder.
2. Integrate googletest and add unit tests.
3. Implement calibration functions of the ADS114S0xB.