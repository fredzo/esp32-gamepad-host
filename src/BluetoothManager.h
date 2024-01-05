
/* Include Guard */

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

/*************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************************************/

/* Functions */

int btstackInit(int maxConnections);
void btstackRun();
void maybeRumble();

/*************************************************************************************************/

#ifdef __cplusplus
}
#endif  /* extern "C" */

#endif
