#ifndef _RELAY_H_
#define _RELAY_H_

typedef enum
{
    ENTER_DISABLE,
    ENTER_ENABLE,
} enter_state;


#define RELAY_ACTION(a) if (a)  \
        GPIO_SetBits(GPIOD,GPIO_Pin_2);\
    else        \
        GPIO_ResetBits(GPIOD,GPIO_Pin_2)

void relay_module_init(void);

#endif
