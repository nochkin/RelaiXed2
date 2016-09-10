The RelaiXed project source code consists of three parts that are largely independent,
and are compiled and linked separately:
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
RelaixedApp-20161010, svn rev. 83:
----------------------------------
 - Completed software-support for a 16x2 OLED display on the I2C bus, works for the RelaiXedPassive.

RelaixedApp-20160809, svn rev. 82:
----------------------------------
 - Support 2 or 3 relay-boards on the RelaixedPassive, providing left-right balance control
   between the boards at i2c bus-address 0 and 1. A further board at 2 is not influenced by balance.
   The latter is obtained by changing the PCB jumper-setting at A0 (JP301) on the mcp23017.

RelaixedApp-20160306, svn rev. 81:
----------------------------------
 - Suppress any reaction on IR buttons '5' and '6' for the (4-input) RelaixedPassive
 - Support separate IR power-up and power-down commands for remotes with such buttons
Uploaded files are the 'RelaixedApp-20160306' which can be inserted through USB,
and the 'RelaixedPicImage-20160306' which includes the USB bootloader, and can be inserted with a PIC programmer.

RelaixedApp-20141104, svn rev. 78:
----------------------------------
 - Bug repair: at volume 00 the input relays must detach for real silence (bug was present since svn 68)

RelaixedApp-20141001, svn rev. 77:
----------------------------------
 - Added support for a 16x2 character OLED display attached to the I2C bus.
   (for now, mostly tested with the RelaixedPassive board.
    There are still some issues with bus reliability at the electrical level...)

RelaixedApp-20140811, svn rev. 73:
----------------------------------
 - Update the project configuration and all source files to use (the free version of)
   Microchips XC8 compiler and MPLAB-X IDE.
 - Added support for the RelaixedPassive target board, recognizing this type at run-time.

RelaixedApp-20130701, svn rev. 64:
----------------------------------
Resolved a terrible bug that I didn't notice myself earlier but was reported by a user:
After a power-up from primary power, sometimes the IR reception entered a non-operational state.
This was repaired, the IR reception now comes up reliably.
Two other updates:
 - Waking up the relaixed from standby mode by pressing its knob occurs now on the 'press'
   (was on the 'release' which was definitely less intuitive.)
 - When switching input channel, there is now an automatic mute for a fraction of a second.
   This prevents a loud 'tick' which could otherwise occur when switching at high volume.

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



