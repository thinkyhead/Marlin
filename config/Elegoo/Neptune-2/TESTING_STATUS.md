# Testing status, known issues and workarounds, and FAQ's

HELP WANTED:

I'll take whatever anyone is interested in helping with

- Testing of features
- Maintaining configurations
- Code reviews 
- Maintaining/Proofreading this content 

## Testing status

The 1.3 BLTouch Version for tne Neptune 2 and 2d are confirmed to be working in this release. 

# Known issues and workarounds

## Homing Failed

If after homing, your display is red and displays

> Homing failed, Printer halted, Please reset 

The issue [#23436](https://github.com/MarlinFirmware/Marlin/issues/23436) is closed and you should no longer encounter this error.

Reboot the printer, then click on *SETTINGS ICON -> Configuration -> NEXT ARROW (>) -> Factory Reset*. You will hear a beep. Go back to the home screen and try to level again. Sometimes this needs to be done a couple of times (twice is the most I have had to try). Once you get the machine homed successfully, this issue does not return unless you reflash the firmware.

## Screen calibration freezes 

After you click the bottom right calibration point, reboot your machine if it prompts you to click the top left again, and you see two calibration touch points on the screen. I have never had to do this more than twice.

## Machine Keeps Rebooting

**Solution:** Remove the SD card with the firmware on it. 

## Pronterface/Otoprint Won't Connect

**Solution:** Ensure your baudrate is set to 250000 or AUTO. If that does not resolve it and you are using a hub. Try connecting directly. 
