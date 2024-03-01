/*
 * timer.h
 *
 *  Created on: Mar 5, 2014
 *      Author: mike.starkey
 */

#ifndef TIMER_H_
#define TIMER_H_

//#include "project.h"
#include <stdint.h>

double TimerMilliseconds();
uint16_t TimerMillisecondElapsed(double start);
uint16_t TimerMillisecondsU16();
void TimerDelay(uint16_t t);

#endif /* TIMER_H_ */
