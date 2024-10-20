#include "../../inc/MarlinConfig.h"
#if BOTH(ANKER_MAKE_API, ANKER_LOG_DEBUG)

void anker_check_block_buf();
void set_anker_debug_flag(int value);
int get_anker_debug_flag();

#endif // ANKER_MAKE_API && ANKER_LOG_DEBUG
