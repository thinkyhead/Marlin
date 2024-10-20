/*
 * @Author       : winter
 * @Date         : 2022-05-13 14:28:04
 * @LastEditors  : winter
 * @LastEditTime : 2022-07-08 12:09:41
 * @Description  :
 */
#include "anker_m_cmdbuf.h"
#include "../../gcode/queue.h"
#include "../../gcode/gcode.h"
#include "../../gcode/parser.h"

#if BOTH(ANKER_MAKE_API, ANKER_M_CMDBUF)

// Change any ABCD in the buffer into XYZE
static void anker_m_cmdbuf_translate_xyze_axis(char *buf) {
  char *token;
  token = strchr(buf, 'A'); if (token) *token = 'X';
  token = strchr(buf, 'B'); if (token) *token = 'Y';
  token = strchr(buf, 'C'); if (token) *token = 'Z';
  token = strchr(buf, 'D'); if (token) *token = 'E';
}

void anker_m_cmdbuf_init() {
  anker_m_cmdbuf_info_t *p_info = get_anker_m_cmdbuf_info();
  p_info->queue_translate_xyze_axis = anker_m_cmdbuf_translate_xyze_axis;
}

anker_m_cmdbuf_info_t *get_anker_m_cmdbuf_info() {
  static anker_m_cmdbuf_info_t anker_m_cmdbuf_info = {0};
  return &anker_m_cmdbuf_info;
}

#endif // ANKER_MAKE_API && ANKER_M_CMDBUF
