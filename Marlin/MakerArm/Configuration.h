/**
 * Which MakerArm are you using?
 */
//#define MAKERARM_CFP            // Lahore's MakerArm
//#define MAKERARM_EVT0_TMC2130   // MakerArm prototype without TMC drivers
//#define MAKERARM_EVT2           // MakerArm prototype without TMC drivers
//#define MAKERARM_EVT2_PLUS      // CNC Test Arm at this writing
//#define MAKERARM_EVT2_TMC2130   // MakerArm prototype for testing
//#define MAKERARM_BETA0          // MakerArm Beta 0 with AB Hall endstops
#define MAKERARM_BETA3          // MakerArm Beta 3
//#define MAKERARM_TEST           // Testing on a Megatronics board

//
// Include the appropriate configuration
//
#if ENABLED(MAKERARM_CFP)
  #include "CFP/Configuration.h"
#elif ENABLED(MAKERARM_EVT0_TMC2130)
  #include "EVT0_TMC2130/Configuration.h"
#elif ENABLED(MAKERARM_EVT2)
  #include "EVT2/Configuration.h"
#elif ENABLED(MAKERARM_EVT2_PLUS)
  #include "EVT2_PLUS/Configuration.h"
#elif ENABLED(MAKERARM_EVT2_TMC2130)
  #include "EVT2_TMC2130/Configuration.h"
#elif ENABLED(MAKERARM_BETA0)
  #include "BETA0/Configuration.h"
#elif ENABLED(MAKERARM_BETA3)
  #include "BETA3/Configuration.h"
#elif ENABLED(MAKERARM_TEST)
  #include "TEST/Configuration.h"
#endif

#ifndef X_CS_PIN
  #define X_CS_PIN         14
#endif
#ifndef Y_CS_PIN
  #define Y_CS_PIN         15
#endif
#ifndef Z_CS_PIN
  #define Z_CS_PIN         30
#endif
#ifndef E0_CS_PIN
  #define E0_CS_PIN        39
#endif
