#ifndef _KEYSCAN_H_
#define _KEYSCAN_H_

#include  "FreeRTOS.h"

#define  KEY_SCAN_PERIOD    30//debunce 30ms

typedef struct
{
    uint8_t row_index;
    uint8_t col_index;
} Key_ValueTypeDef;

void keyscan_module_init(void);

#endif
