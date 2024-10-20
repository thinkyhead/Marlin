/*
 * @Author       : winter
 * @Date         : 2022-05-13 14:28:19
 * @LastEditors  : winter
 * @LastEditTime : 2022-07-08 12:08:20
 * @Description  :
 */
#ifndef __ANKER_M_CMDBUF_H__
#define __ANKER_M_CMDBUF_H__

#include "../../inc/MarlinConfig.h"

#if BOTH(ANKER_MAKE_API, ANKER_M_CMDBUF)

typedef struct {
  void (*queue_translate_xyze_axis)(char *buf);
} anker_m_cmdbuf_info_t;

void anker_m_cmdbuf_init(void);
anker_m_cmdbuf_info_t *get_anker_m_cmdbuf_info(void);

#endif // ANKER_MAKE_API && ANKER_M_CMDBUF
#endif // __ANKER_M_CMDBUF_H__
