

#define X_ENABLE_PIN		38
#define Y_ENABLE_PIN 		62
#define Z_ENABLE_PIN		56
#define E0_ENABLE_PIN		24
#define CS_X				46
#define CS_Y				49
#define CS_Z				48
#define CS_E0				47
#define TEMP_0_PIN			13	//ANALOG NUMBERING
#define TEMP_1_PIN 			15	//ANALOG NUMBERING
#define TEMP_BED_PIN		14	//ANALOG NUMBERING
#define HEATER_BED_PIN		8
#define FAN_PIN				9
#define HEATER_0_PIN		10
#define I2C_SCL       		21
#define I2C_SDA      		20

  #ifdef NUM_SERVOS
    #define SERVO0_PIN         11

    #if NUM_SERVOS > 1
      #define SERVO1_PIN         6
    #endif

    #if NUM_SERVOS > 2
      #define SERVO2_PIN         5
    #endif

    #if NUM_SERVOS > 3
      #define SERVO3_PIN         4
    #endif
  #endif
