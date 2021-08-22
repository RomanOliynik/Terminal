#ifndef TERMINAL_H
#define TERMINAL_H

#include "stm32g0xx_hal.h"

void terminal_init(void);


void terminalParse(char *command, int command_size);


#endif

