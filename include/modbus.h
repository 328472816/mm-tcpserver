#ifndef __MODBUS_H
#define __MODBUS_H
#include "common.h"


#define REG_MASTERFD   0
#define REG_SIZE   10
int cmd_get(char *buf,int len,int fd);
#endif