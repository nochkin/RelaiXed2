This is a modified version of the Microchip bootloader:
- adapted to the Relaixed front-PCB and its IO-signals, but more importantly,
- adapted to operate really interrupt-driven.
The USB stack is not 'polled' anymore every millisecond as in Microchips example projects.
Also, the USB stack is shared bwteen the bootloader and the Relaixed development/debug runtime.

This project can be compiled with the free ('Lite') version of Microchips C18 C-compiler,
together with Microchips' assembler and linker. This is probably best done by installing
(integrating) the C18 compiler in their MPLAB SDE.

Jos van Eijndhoven