
/* Include Guard */

#ifndef GAMEPAD_MANAGER_H
#define GAMEPAD_MANAGER_H

/*************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************************************/

/* Libraries */

#include <inttypes.h>

/*************************************************************************************************/

/* Functions */

void gamepad_handler(uint8_t* hid_report_data, const uint16_t report_len);

/*************************************************************************************************/

#ifdef __cplusplus
}
#endif  /* extern "C" */

#endif
