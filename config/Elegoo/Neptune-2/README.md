** NOTICE: ** Several people are still testing this. If you want something more stable, you may want to wait. You can see the [current testing status, known issues, workarounds, and faq's here](https://github.com/just-trey/Marlin/blob/elegoo-neptune-2/config/Elegoo/Neptune-2/TESTING_STATUS.MD).

# Marlin 2.x - Color UI - 3D Printer Firmware compiled for Elegoo Neptune 2

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

**Note:** The configurations and compiled versions are both available in this bundle. 

This is a package of Marlin 2.x configurations for the Elegoo Neptune 2. The configurations are compatible with the Elegoo Neptune 2S. The source is a fork from [Marlin 2](https://github.com/MarlinFirmware/Marlin/releases). No changes have been made to the Marlin codebase, and only configuration files for the Elegoo Neptune 2 are added. 

These files are being provided as opensource under the same [GPL v3](https://www.gnu.org/licenses/gpl-3.0) license as Marlin 2.

## Please Read and Consider:
I am providing these configurations to the community for free. If you find this helpful or your ETSY printer farm benefits from this (or makes you rich :grinning:), I would like you to please consider donating to a nonprofit close to my heart that supports my community. More information is at the end of this [README](#donation-request). 

# What are the features I get from using this?

All versions provided come with these features.

1. A safe version of alternative firmware.
   - I found a few configurations online and in repos that were not appropriately configured (Thermal runway protection was off or busted) 
1. An alternative UI
   - Not as "pretty", but much more helpful
   - Update many configuration values (including ESTEPS) right on the screen
   - PID Autotuning for the nozzle AND bed from the UI.  
1. [S-Curve Acceleration](https://github.com/synthetos/TinyG/wiki/Jerk-Controlled-Motion-Explained)
1. ARC Support (I am not sure this is new or not)
1. **M117** Change filament at layer.
1. The progress screen will update if you are printing from Octoprint.

BLTouch versions also include:
1. Z Level Offset Wizard
1. **Live Z** Support
1. Bad tramming wizard option (I mean, you still need to level every once and a while)

# Do I lose any features if I use this?

The following items are not supported:

1. If you have a WIFI Card, it WILL NOT WORK (MKS made that call, not me) 
1. Icon previews do not exist in UI

# Setup and install

## Getting things ready

1. Download this [zip](https://github.com/just-trey/Marlin/releases/download/Marlin-2.0.9.3-elegoo/Elegoo.zip) file.
2. extract the zip file on your computer
3. Copy the desired `elegoo.bin` configuration to your SD Card. No other files are needed to install.  
   
   ***NOTE:*** **IT IS IMPORTANT YOU KNOW THE VERSION OF THE ZNP ROBIN NANO BOARD YOU HAVE, AND YOU NEED TO USE THAT VERSION OF THE FIRMWARE**
  
   - The directory structure is as follows:

     ![Screenshot from 2022-01-02 01-15-06](https://user-images.githubusercontent.com/10281380/147867833-04257d55-69a0-4653-8c9d-c61f433bf567.png)  
   - All firmware and configurations for 1.2 boards are in the 1.2 directory
   - All firmware and configurations for 1.3 boards are in the 1.3 directory
   - If you *DO NOT* have a BLOTUCH, use the file in the STOCK directory
   - If you *DO* have a BLOTUCH, use the file in the BLOTUCH directory

## Install

1. Power off your printer
1. Put the sd card in with the appropriate `elegoo.bin` file... *Reminder: Did I mention to make sure you have the correct version for your board?*
1. Turn on your printer.
1. The firmware will install.
1. `I'm not sure if you get this EEPROM warning here` - You may get an EEPROM version warning (can someone send me a screenshot?) If you do, click the checkmark.
1. Once installed, click on the calibration touchpoints. When the touch calibration is complete, it will display a confirmation message. You may think it froze but give it a little time.

   Please note: After you click the bottom right calibration point, reboot your machine if it prompts you to click the top left again. I have never had to do this more than twice. 

1. `Or you may get the EEPROM warning here` - need confirmation`

## What to do next
**Confirm you can home the machine** this is a known issue with a workaround 

1. Click in the middle of the screen around the Y and the homing screen will display.
1. Click on the Home icon.
1. Did you get the RED Error Screen?
   - Yes? - [How to resolve is here](TESTING_STATUS.MD#homing-failed).
   - No? - I'm jealous. Have fun!

## (Optional, but recommended) PID AUTOTUNE your machine.

*Did you know?* - Elegoo ships with some default values for this, but they can vary with *every* machine. PID tuning is a safety feature, and unless you want my machine's values, I suggest you do this.

1. From the main screen click on *settings icon -> Configuration -> Advanced Settings -> NEXT ARROW (>) -> Tempeture -> PID AUTOTUNE E1*
1. The nozzle will heat up and down for a few minutes. When completed, the screen will display a confirmation message on the bottom of the status screen.
1. From the main screen click on *settings icon -> Configuration -> Advanced Settings -> NEXT ARROW (>) -> Tempeture -> NEXT ARROW (>) -> PID AUTOTUNE BED*
1. The bed will heat up and down for a few minutes. When completed, the screen will display a confirmation message on the bottom of the status screen.
1. From the main screen click on the *settings icon -> Configuration -> NEXT ARROW (>) -> Store settings*

## Donation Request

Please consider donating to support the Tyler Wentz Memorial Fund if you find this helpful.

[![Donate](https://user-images.githubusercontent.com/10281380/147870647-23f016bf-29f9-41ec-a78f-81ef9633ff23.png))](https://golf.twentz.com/donations)

My son, Tyler Wentz, passed away after having a bodysurfing accident in 2015. We established a 501(c) (3) nonprofit organization to raise money to provide two scholarships in our community to deserving students. We are 100% volunteer-driven, and 100% of donations (less the 2.7% Square Processing Fee) will go straight towards supporting the scholarships. 

You can learn more about the Tyler Wentz Memorial Fund fund at: https://twentz.com/ 

---

Have fun click aroudn the interface let me knw if you have any issues and Happy Printing!
