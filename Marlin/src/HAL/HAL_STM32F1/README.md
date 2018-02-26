# This HAL is for STM32F103 boards used with libmaple/stm32duino Arduino core.

# This HAL is in development. Currently has been tested in Malyan M200 (103CBT6), Chitu 3d (103ZET6), and custom boards(103VET6).

### The stm32 core needs a modification in the file util.h to avoid conflict with Marlin macros for Debug.
Since only 1 file needs change in the stm32duino core, it's preferable over making changes to Marlin.


After these lines:
<>
#else
#define ASSERT_FAULT(exp) (void)((0))
#endif
<>

Add the following 3 lines:
<>
#undef DEBUG_NONE
#undef DEBUG_FAULT
#undef DEBUG_ALL
<>

### stm32duino core needs a modification for pre-processor macros to work correctly with the pins:
GPIO Pins are defined in an ENUM in the stm32duino/libmaple core.
Due to that, they all evaluate as 0 to the pre-processor Macros. Result: some Macros don't work right unless you modify the board.h file in your variant (STM32F1/variants/generic_stm32f103v/board in my case using a VET MCU).
So comment out the enum there, and define all your pins with macros. They need to be in the same order as they are in the ENUM:

/*enum {
PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7,PA8,PA9,PA10,PA11,PA12,PA13,PA14,PA15,
PB0,PB1,PB2,PB3,PB4,PB5,PB6,PB7,PB8,PB9,PB10,PB11,PB12,PB13,PB14,PB15,
PC0,PC1,PC2,PC3,PC4,PC5,PC6,PC7,PC8,PC9,PC10,PC11,PC12,PC13,PC14,PC15,
PD0,PD1,PD2,PD3,PD4,PD5,PD6,PD7,PD8,PD9,PD10,PD11,PD12,PD13,PD14,PD15,
PE0,PE1,PE2,PE3,PE4,PE5,PE6,PE7,PE8,PE9,PE10,PE11,PE12,PE13,PE14,PE15,
};*/
/* Note PB2 is skipped as this is Boot1 and is not going to be much use as its likely to be pulled permanently low */

#define PA0	0
#define PA1	1
#define PA2	2
#define PA3	3
#define PA4	4
#define PA5	5
#define PA6	6
#define PA7	7
#define PA8	8
#define PA9	9
...


### Main developers:
Victorpv
xC000005


### Most up to date repository for this HAL:
https://github.com/victorpv/Marlin/tree/bugfix-2.0.x

PRs should only be sent to Marlin bugfix-2.0.x branch once tested in printing so not to introduce new bugs.
For testing/dev, you can submit to the above branch


