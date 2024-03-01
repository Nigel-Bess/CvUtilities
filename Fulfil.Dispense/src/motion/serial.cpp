/*
 * serial.c
 *
 *  Created on: Jul 16, 2018
 *      Author: mike.starkey
 */


#include "Fulfil.Dispense/motion/serial.h"
#include "Fulfil.Dispense/motion/motion_timer.h"
#include <stdint.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
//#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

//eclipse g++ doesn't have definition for FIONREAD but Arm/Linux crosstool does
#ifndef FIONREAD
	#define FIONREAD 0x541B
#endif

int16_t SerialWrite(const uint8_t* buffer, uint16_t length, int fd) {
	int16_t bytesWritten = 0;
	bytesWritten = write(fd, buffer, length);
	if(bytesWritten < length)
		printf("SerWrite failed %d", length);
//	if(bytesWritten > 0){
//		printf("Wrote: ");
//		for(int i = 0; i < bytesWritten; i++)
//			printf("%02X ", buffer[i]);
//		printf("\n");
//	}

    return bytesWritten;
}

int16_t SerialRead(uint8_t* buffer, uint16_t length, int fd) {
	int16_t bytesRead = 0;
	bytesRead = read(fd, buffer, length);

//	if(bytesRead > 0){
//		printf("Read: ");
//		for(int i = 0; i < length; i++)
//				printf("%02X ", buffer[i]);
//			printf("\n");
//	}
    return bytesRead;
}

int BytesToRead(int fd)
{
	int bytes=0;;
	ioctl(fd, FIONREAD, &bytes);
    return bytes;
}
void SerialClear(int fd){
	tcflush(fd, TCOFLUSH);
	tcflush(fd, TCIFLUSH);
}

speed_t GetBaudrate(uint32_t baudrate){
	switch(baudrate) {
		case 0: return B0;
		case 50: return B50;
		case 75: return B75;
		case 110: return B110;
		case 134: return B134;
		case 150: return B150;
		case 200: return B200;
		case 300: return B300;
		case 600: return B600;
		case 1200: return B1200;
		case 1800: return B1800;
		case 2400: return B2400;
		case 4800: return B4800;
		case 9600: return B9600;
		case 19200: return B19200;
		case 38400: return B38400;
		case 57600: return B57600;
		case 115200: return B115200;
		case 230400: return B230400;
		case 460800: return B460800;
		case 500000: return B500000;
		case 576000: return B576000;
		case 921600: return B921600;
		case 1000000: return B1000000;
		case 1152000: return B1152000;
		case 1500000: return B1500000;
		case 2000000: return B2000000;
		case 2500000: return B2500000;
		case 3000000: return B3000000;
//		case 3500000: return B3500000;
//		case 4000000: return B4000000;
		default: return B9600;
	}
}

int ComOpen(uint32_t baud, const char* port){
	struct termios port_settings;
	int16_t code;
	speed_t br; br = GetBaudrate(baud);
	int descriptor = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);// | O_NDELAY);

	memset(&port_settings, 0, sizeof(struct termios));

	cfsetispeed(&port_settings, br);    // set baud rates
	cfsetospeed(&port_settings, br);
	//port_settings.c_iflag 	  &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
	port_settings.c_cflag     &=  ~PARENB;        // Make 8n1
	port_settings.c_cflag     &=  ~CSTOPB;
	port_settings.c_cflag     &=  ~CSIZE;
	port_settings.c_cflag     |=  CS8;

	port_settings.c_cflag     &=  ~CRTSCTS;       		// no flow control
	//port_settings.c_cc[VMIN]      =   0;                  // read doesn't block
	//port_settings.c_cc[VTIME]     =   1	;
	port_settings.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	tcflush(descriptor, TCOFLUSH);
	tcflush(descriptor, TCIFLUSH);
	tcsetattr(descriptor, TCSANOW, &port_settings);

	return descriptor;
}

int ComDisconnect(int descriptor){
	return close(descriptor);
}