#pragma once

#include "../../inc/ANKER_Config.h"

extern "C" {

#include <stdint.h>

void oci_init();

void oci_latch_clear();

void hw_select();

uint8_t hw_ver_read();

void hotbed_mos_err_polling();

extern uint8_t fatal_err;

}
