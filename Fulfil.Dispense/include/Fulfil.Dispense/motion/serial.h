/*
 * serial.h
 *
 *  Created on: Mar 5, 2014
 *      Author: mike.starkey
 */

#ifndef SERIAL_H_
#define SERIAL_H_

//#include "project.h"
#include <termios.h>
#include <stdint.h>

int16_t SerialWrite(const uint8_t* buffer, uint16_t length, int fd);
int16_t SerialRead(uint8_t* buffer, uint16_t length, int fd);
int ComOpen(uint32_t baud, const char* port);
int BytesToRead(int fd);
speed_t GetBaudrate(uint32_t baudrate);
void SerialClear(int fd);
int ComDisconnect(int descriptor);
int USBConnect(uint32_t baud);

#endif /* SERIAL_H_ */
