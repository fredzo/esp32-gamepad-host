
/* Include Guard */

#ifndef BT_MANAGER_H
#define BT_MANAGER_H

/*************************************************************************************************/

/* C++ compiler compatibility */

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************************************/

/* Functions */

int btstack_main(int argc, const char * argv[]);
int btstack_main_new(int argc, const char * argv[]);
void maybeRumble();
void btstack_run(void);

/*************************************************************************************************/

#ifdef __cplusplus
}
#endif  /* extern "C" */

#endif
