# Testing status, known issues and workarounds, and FAQ's

HELP WANTED:

I'll take whatever anyone is interested in helping with

- Testing of features
- Maintaining configurations
- Code reviews 
- Maintaining/Proofreading this content 

## Testing status

The follwing items have not been tested in any configuration:

  - Filamanet runout detection
  - Resume on power failure

Currenlty only the v1.3 STOCK and v1.3 BLTOUCH have been installed and confirmed to be working

# Known issues and workarounds

## Homing Failed

If after homing your display is red and displays

> Homing failed, Printer halted, Please reset 

Reboot the printer, then click on *settings icon -> -> -> ->*
## Screen calibration freezes 

After you click the bottom right calibration point, reboot your machine if it prompts you to click the top left again and you see 2 calibraiton touch points on the screen. I have never had to do this more than twice. 

## Machine Keeps Rebooting

Solution: Remove the SD card with the firmware on it. 

## Pronterface/Otoprint Won't Connect

Solution: Ensure your baudrate is set to 250000 or AUTO. If that does not resolve it and you are using a hub. Trey connecting directly. 

# FAQ's

Q. Why only one file? What happend to the `elegoo.txt` file and the fonts and images?

A. Marlin 2 does not use any addional files for the provided configurations. IF you were to compile using LVGL you would get an `assets` folder as well. 


