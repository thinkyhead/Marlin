TRAMS Firmware
==============

Development History
-------------------

**V0.91**
- Cleaned up, removed comments and unused lines
- TRAMS has its own board configuration
- Homing uses defines from Configuration.h

**V0.92**
- Smoother circles and corners

**V0.93**
- StallGuard2 added. ATTENTION: still in beta phase
- In configuration.h you can now choose the homing direction and the homing switch (left/right)

**V1.00 - 09.06.2016**
- In general, tidied up, tested and made it easier to configure.
- SPI communication refactored, homing and 5130 functions to TMC5130.c, stepper.c documented, configuration in TMC_TRAMS_CONFIGURATION.h, pins in TMC5130.h.
- Tested with Eclipse-Mars2 + Arduino plug-in: click on "add libraries to the project" and include: LiquidCrystal, LiquidCrystal_I2C, LiquidTWI2, SPI, U8glib and Wire (if not present by default, download from internet).