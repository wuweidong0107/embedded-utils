#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "aw5808.h"

int devices_init(void);
void devices_exit(void);
aw5808_t *get_aw5808(int index);

#endif