This is a modified version of the Microchip bootloader:
- adapted to the Relaixed front-PCB and its IO-signals, but more importantly,
- adapted to operate really interrupt-driven.
The USB stack is not 'polled' anymore every millisecond as in Microchips example projects.
Also, the USB stack is shared bwteen the bootloader and the Relaixed development/debug runtime.

This project can be now compiled with the free version of Microchips XC8 C-compiler,
and contains a project setup for Microchips MPLAB-X IDE. This is updated from the previous
project version that relied on (the free version of) Microchips C18 C-compiler and their MPLAB SDE.

Jos van Eijndhoven