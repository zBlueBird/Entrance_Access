#ifndef _SIGNAL_CHECK_H_
#define _SIGNAL_CHECK_H_

typedef enum {
	CAR_STATE_STOP,
	CAR_STATE_FORWARD,
	CAR_STATE_BACKWARD,
	CAR_STATE_RIGHT,
	CAR_STATE_LEFT,
}car_state;

void signal_module_init(void);

car_state signal_get_state(void);

#endif

