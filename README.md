Pico Good Box
=============

This is a mashup of external projects:
- [Microshell](https://github.com/marcinbor85/microshell)
- [picoprobe](https://github.com/raspberrypi/picoprobe)
- [SUMP logic analyzer](https://github.com/perexg/picoprobe-sump) + [random cdc_sump.h](https://github.com/PoroCYon/picoprobe-sump)
- [Sigrock logic analyzer](https://github.com/gamblor21/rp2040-logic-analyzer)
- [embedded-log](https://github.com/to9/embedded-log)

Currently they barely stick together using some Superglue and Duck Tape. The Plan from main.cpp:

```
// USB CDC0 (user text console)
// USB CDC1 (user binary console)
// USB CDC2 (defaults to UART0 bridge)
// USB CDC3 (defaults to UART1 bridge)
// USB CDC4 (defaults to SUMP logic analyser)
// USB CDC5 (defaults to local log)
// USB VENDOR (raw data application: defaults to picoprobe)
// UART0 is stdio, available for target device on request from console
// UART1 available for target device
```

Microshell: command prompt
==========================
( https://github.com/marcinbor85/microshell )


Picoprobe: SWD debugger for ARM
===============================
( https://github.com/raspberrypi/picoprobe )


Logic Analyzer: SUMP protocol
=============================
( https://github.com/perexg/picoprobe-sump )

Supported features:
    
- up to 16 probes (gpio 6-21 by default)
- 200kB RAM for samples
- RLE encoding
- test mode (external pattern)
  * probe 0 and 1 - 10Mhz PWM
  * probe 1 and 2 - 1Mhz PWM
  * probe 3 and 4 - 1kHz PWM
  * probe 8 and 9 - 1kHz PWM (swapped levels)
- test pin (gpio 22) - 5Mhz 50%/50% PWM for probe tests (activated only when sampling)
    
Limits:
    
- 50Mhz sampling rate when compiled with the TURBO_200MHZ define (otherwise 31.25Mhz)
- basic triggers are implemented (functional up to 10Mhz - TODO: PIO support)
    
This protocol is supported in sigrok as openbench-logic-sniffer:
    
```
  pulseview --driver=ols:conn=/dev/ttyACM1
    
  sigrok-cli --driver=ols:conn=/dev/ttyACM1 --config samplerate=50Mhz \
             --config pattern=External --samples 256 --channels 0-1
```

Compile:

```
  mkdir build
  cd build
  TURBO_200MHZ=1 PICO_SDK_PATH=/opt/pico-sdk cmake ..
```

Compile RAM code (debug):

```
  mkdir build
  cd build
  TURBO_200MHZ=1 PICO_SDK_PATH=/opt/pico-sdk cmake -DPICO_NO_FLASH=1 -DCMAKE_BUILD_TYPE=Debug ..
```
    
Misc:
    
- picoprobe reset pin is on gpio 28 (instead 6) now

[Link to protocol](https://www.sump.org/projects/analyzer/protocol) | 
[Link to libsigrok](https://github.com/sigrokproject/libsigrok/tree/master/src/hardware/openbench-logic-sniffer)


Logic Analyzer: sigrock
=======================
( https://github.com/gamblor21/rp2040-logic-analyzer )

This project modified the PIO logic analyzer example that that was part of the 
Raspberry Pi Pico examples. The example now allows interactive configuration 
of the capture to perform and outputs the capture in CSV format suitable for
importing into sigrock / Pulseview for further analysis.

To use the analyzer install it on a Pico and connect to the COM port at 921600 
baud. Once connected press h to get help of the commands. The capture is
only limited by the abilities of the Pico.

The commands are:
  * p# - Set the first pin to receive capture data
  * n# - Set how many pins to receive capture data
  * f# - Set the freqency to capture data at in Hz
  * t(1)(0) - Set the trigger to high or low. Trigger happens off first pin
  * s# - Set how many samples to capture
  * g - Go!

Once "go" is selected the trigger will arm and wait for the specified signal.
The output is a CSV file, each line contains every pin being sampled. The output
can be saved with any program that can read a serial port to a file. Just be
aware a large number of samples can take quite a while to transfer. The
onboard LED will blink as the transfer is happening so you can know when to end
the save.
