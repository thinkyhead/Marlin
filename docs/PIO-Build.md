# PlatformIO Build

PlatformIO is an incredible project that allows developers to build applications targeting many different processors and boards, from tiny embedded chips with Arduino to lightweight RTOSes and full operating systems, allowing a single project to target any combination of platforms.

With PlatformIO handling the routing of the build to the correct tools and libraries, Marlin is able to target hundreds of boards and even build as a simulator that runs right on your desktop (bringing us one step closer to a 3D printing GPIO peripheral).

## PIO Build Files

The core configuration for PlatformIO is the `platformio.ini` file which tells PlatformIO where to find the other INI files and provides the common settings for all Marlin builds.

Some PlatformIO builds invoke our custom scripts stored in `buildroot/share/PlatformIO/scripts`.

These scripts sometimes depend on data stored in source files in the Marlin project.

- `boards.h` : Contains the list of boards supported by Marlin with short descriptions.
- `pins.h` : Includes the appropriate pins file for the selected `MOTHERBOARD` with structured comments indicating the build environments pertaining to each board.

## Build Process

1. Use `platformio.ini` to filter out all source files except the absolute minimal requirements, augmented by a script that recurses into `build_src_filter` folders to more completely filter the contents.
2. In `common-dependencies.py` pre-build Marlin with the best available version of GCC to figure out which features are enabled. Parse `features.ini` to bring in required source files and folders based on enabled features. This cuts the build time down considerably.
