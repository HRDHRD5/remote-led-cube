# Controll your 8x8x8 LED-Cude with a raspberry pi pico
This Repository contains the software for the RPI pico (pi-software/led-cube-controller) and animation client software (pi-software/).

# Demo video running a clock on the cube
https://github.com/user-attachments/assets/8a61950b-e3a3-4856-b92e-824cd2bd9148

# Beware
## Requirements
1.
The Cube must run either the original 8x8x8-LED Cube **Firmware by Sliicy** (https://github.com/Sliicy/8x8x8-LED) based on tomazas firmware (https://github.com/tomazas/ledcube8x8x8) or my modified Version located in [cube-firmware](https://github.com/HRDHRD5/remote-led-cube/tree/main/cube-firmware).
I highly recommend the modified version, as I optimized the firmware and also fixed a bug that caused a lot of trouble for me.

The firmware by Sliicy is also present in this repository in [cube-firmware](https://github.com/HRDHRD5/remote-led-cube/tree/main/cube-firmware).

You can flash the firmware with stcgal (https://github.com/grigorig/stcgal) on linux or with the original cube flashing tool as described by tomazas in https://github.com/tomazas/ledcube8x8x8.

2.
Since the RPI Pico runs UART only on 3.3V and the Cube expects 5.5V you need a level Shifter. Lukily the pico has a 5V VCC, so the shifter can be build with two 2N7000 Mosfets:

![schematic for 3.3V to 5V Level Shifter](LevelShifter.png)

Please note that this is just the solution i took and the level shifter can be build totally different.
