#ifndef __LINUX_HALL_SENSOR_H
#define __LINUX_HALL_SENSOR_H

#define HALL_POLE_BIT  1
#define HALL_POLE_N	  0
#define HALL_POLE_S	  1
#define HALL_FAR 	  0
#define HALL_NEAR	  1

extern struct blocking_notifier_head hallsensor_notifier_list;

extern int register_notifier_by_hallsensor(struct notifier_block *nb);
extern int unregister_notifier_by_hallsensor(struct notifier_block *nb);
int hallsensor_enable_by_hover_driver(int on);
#endif

