# ADS129X Arduino library

This library adds support for the ADS129X series of analog front-ends from Texas Instruments to the Arduino environment. It is based on work by [conorrussomanno](https://github.com/conorrussomanno/ADS1299) and forked from 
[Ferdinand Keil](https://github.com/ferdinandkeil/ADS129X).

The library has been tested on both ADS1298, ADS1294 and ADS1299 and ADS1299-6 chips.

## Modes of operation

The library support two modes of operation: interrupt-driven and polling. Both modes use the same API and your sketch still has to poll for available data even when in interrupt mode. However in interrupt mode the response time to changes on the *DRDY* line is much quicker and thus higher sample-rates are supported and tested on the Teensy 3.2, up to 16ksps.  The Teensy 4.0 has been tested and will allow running at the full 16ksps for ADS1299 and up to the full 32ksps for ADS1298 in both interrupt and polling mode.

Interrupt mode is the default, to switch to the old polling operation add this to your sketch as the first line:

```arduino
#define ADS129X_POLLING
```

When multiple devices share the SPI bus you may want to use polling mode as not to interfere with the SPI transactions of other devices.  However, using the Teensy 4.0 should be capable of using iterrupt mode for all devices on the SPI bus.

## Example sketches

Two example sketches are included, Serial_EMG and BTLE_EMG. One transfers the data to a PC via a serial connection, the other uses a nRF8001 BTLE chip to send it to a phone. Both were tested using a custom board including an Olimex nRF8001 breakout and a Teensy 3.1.

Libraries required by the sketches:
* [github.com/PaulStoffregen/Adafruit_nRF8001/](https://github.com/PaulStoffregen/Adafruit_nRF8001/tree/bb385aed176389d806016617a18d7e347074bc3c)
* [github.com/duff2013/LowPower_Teensy3/](https://github.com/duff2013/LowPower_Teensy3/tree/721e3bcab47cc7cdb03f0aee9e11d47611430aa9)

The Serial_OBCI example has been updated to use the ADS1299 chip and the Teensy 4.0.   This was tested on a reference hardware board from the [Biopotential Signal Library](https://hackaday.io/project/5809-biopotential-signal-library) which is shared on the [OSH Park](https://oshpark.com/shared_projects/eg40Tvcp) website and available for purchase for $13 for 3 boards.

![Reference Hardware](https://github.com/rlong19/ADS129X/blob/master/ADS1299_Teensy4.jpg)

License
-------

See [LICENSE](LICENSE.md)
