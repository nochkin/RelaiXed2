Modify the Microchip bootloader, so that it operates interrupt-driven,
not by the simplistic calling of its entry function every millisecond.
We want to re-use the USB stack in the bootloader during Relaixed operation,
driven by interrupts, because 'normally' their will be no USB activity at all.
