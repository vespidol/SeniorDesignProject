/*
 * joystick_driver.h
 *
 * Created: 12/1/2020 00:04:54
 *  Author: vespi
 */ 


#ifndef JOYSTICK_DRIVER_H_
#define JOYSTICK_DRIVER_H_

#include <asf.h>

void joystick_init();
void GetJoystickCoordinates(uint16_t *x_coord, uint16_t *y_coord);


#endif /* JOYSTICK_DRIVER_H_ */