#ifndef _HCSR505_H_
#define _HCSR505_H_

typedef enum
{
    PEOPLE_LEAVING = 0,
    PEOPLE_ARRIVING,
} state;

void hcsr505_module_init(void);

#endif
