# Testing status, known issues and workarounds, and FAQ's

HELP WANTED:

I'll take whatever anyone is interested in helping with

- Testing of features
- Maintaining configurations
- Code reviews 
- Maintaining/Proofreading this content 

## Testing status

The following items have not been tested in any configuration:

  - Filamanet runout detection
  - Resume on power failure

1.3 STOCK, v1.3 BLTOUCH, v1.2 BLTOUCH have been installed and confirmed to load.

# Known issues and workarounds

## Homing Failed

If after homing, your display is red and displays

> Homing failed, Printer halted, Please reset 

Reboot the printer, then click on *SETTINGS ICON -> Configuration -> NEXT ARROW (>) -> Load Settings*. You will hear a beep. Go back to the home screen and try to level again. Sometimes this needs to be done a couple of times (twice is the most I have had to try). Once you get the machine homed successfully, this issue does not return unless you reflash the firmware.

This is being traked at [#23436](https://github.com/MarlinFirmware/Marlin/issues/23436) in the Marlin base repository. 

## Screen calibration freezes 

After you click the bottom right calibration point, reboot your machine if it prompts you to click the top left again, and you see two calibration touch points on the screen. I have never had to do this more than twice.

## Machine Keeps Rebooting

**Solution:** Remove the SD card with the firmware on it. 

## Pronterface/Otoprint Won't Connect

**Solution:** Ensure your baudrate is set to 250000 or AUTO. If that does not resolve it and you are using a hub. Try connecting directly. 