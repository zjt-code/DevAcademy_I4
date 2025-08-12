#ifndef MY_LBS_H
#define MY_LBS_H

typedef void (*led_cb_t)(bool state);
typedef int (*button_cb_t)(void);

struct my_lbs_cb  {
	led_cb_t led_cb;
	button_cb_t button_cb;
};

void my_lbs_init(struct my_lbs_cb *cb);
int my_lbs_send_button_state_indicate(bool button_state);
int my_lbs_send_sensor_notify(uint32_t sensor_value);

#endif