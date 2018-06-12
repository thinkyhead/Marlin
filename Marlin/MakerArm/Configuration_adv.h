#if ENABLED(MAKERARM_CFP)
  #include "CFP/Configuration_adv.h"
#elif ENABLED(MAKERARM_EVT2)
  #include "EVT2/Configuration_adv.h"
#elif ENABLED(MAKERARM_EVT2_PLUS)
  #include "EVT2_PLUS/Configuration_adv.h"
#elif ENABLED(MAKERARM_EVT2_TMC2130)
  #include "EVT2_TMC2130/Configuration_adv.h"
#elif ENABLED(MAKERARM_BETA0)
  #include "BETA0/Configuration_adv.h"
#elif ENABLED(MAKERARM_BETA3)
  #include "BETA3/Configuration_adv.h"
#elif ENABLED(MAKERARM_TEST)
  #include "TEST/Configuration_adv.h"
#endif
