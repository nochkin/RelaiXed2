The RelaiXed project source code consists of three parts that are largely independent,
and are compiled and linked seperately:
1) The ‘RelaixedApp.hex’  embedded software (‘firmware’), stored internally in the PIC microcontroller
   controls the RelaiXed operation. It is stored in the PIC flash memory at addresses above 2000(hex).
   This software is all newly written for this Relaixed2 design.

2) The 'RelaixedHidBootloader.hex' embedded software. This software operates the PIC USB connection and
   contains a self-programmer function to allow re-programming the ‘RelaixedApp’ (1)
   through a USB connection to a PC. This allows Relaixed users to easily update the Relaixed firmware.
   This bootloader is stored in the PIC flash memory at addresses below 2000(hex).
   This software is adapted/extended from example source code provided by Microchip.

3) The PC-bound software application ‘Relaixed.exe’ that is used to communicate with the
   Relaixed microcontroller through a USB cable, mainly for reprogramming the ‘RelaixedApp’.
   Next for programming, it has an additional feature for relaixed application programmers:
   it supports logging of run-time debug messages issued from the RelaixedApp in a PC window.
   This software is adapted/extended from example source code provided by Microchip.

These three parts are in a combined SVN repository that is hosted from Sourceforge:
https://relaixed2.svn.sourceforge.net/svnroot/relaixed2 


Revision History of (1) RelaiXedApp:
====================================


RelaiXedApp-20120105, svn rev. 54:
----------------------------------
Added feature to accelerate the volume-steps when the IR-remote button stays pressed.
Added feature to control a 4-input DAC, merged with the 5 remaining analog input select.
Repaired bug that causes an RC5-protocol remote to skip every other keypress after 'freezing' its use.
Repaired bug that causes a too low voltage drive on the PIC left/right display segment select.

RelaiXedApp-20110818, svn rev. 51:
----------------------------------
Added the initially missing functionality to silence the volume-transition clicks.


RelaiXedApp-20110518, svn rev. 50:
----------------------------------
First version that was distributed.



