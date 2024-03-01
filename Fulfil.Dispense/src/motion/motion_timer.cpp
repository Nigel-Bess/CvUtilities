/*
 * timer.c
 *
 *  Created on: Jul 18, 2018
 *      Author: mike.starkey
 */

#include <time.h>
#include "Fulfil.Dispense/motion/motion_timer.h"
#include <unistd.h>
#include <stdio.h>

//return the current time in ms
double TimerMilliseconds(){

  struct timespec start;
  double elapsed = 0;
  int ret= clock_gettime(CLOCK_MONOTONIC, &start);

  elapsed = start.tv_sec + (start.tv_nsec / 1000000000.0);
  elapsed = elapsed * 1000;

  //registers.RtcTicks = (uint32_t)(elapsed);  //Unclear why this was used in past versions of this code
  return (elapsed);
}

uint16_t TimerMillisecondsU16(){
  double u = TimerMilliseconds();
  uint16_t time = (uint16_t)(((int)u) & 0xFFFF);
  return time;
}

//return the time elapsed since the provided start time input
uint16_t TimerMillisecondElapsed(double start){
  double t = TimerMilliseconds();
  double elapsed = t - start;
  if(elapsed < 0){
    printf("%f - %f \n", t, start);
  }
  return elapsed > 0 ? (uint16_t)elapsed : 0xFFFF;
}


void TimerDelay(uint16_t t) {
  double t0 = TimerMilliseconds();
  while(TimerMillisecondElapsed(t0) < t){
    usleep(1000);
  }
}





