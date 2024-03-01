/*
 * trinamic.cpp
 *  Created on: Jul 30, 2018, mike. Modified Oct 2021, Steve.
 */

#include <iostream>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/motion/trinamic.h"


using fulfil::utils::Logger;
using fulfil::dispense::motion::Trinamic;

Trinamic::Trinamic(uint32_t baud, const char* com_port)
{
  Logger::Instance()->Debug("Initializing Trinamic Motion Controller object now");
  this->baud = baud;
  this->com_port = com_port;
}

bool Trinamic::ConnectMotor()
{
  this->timeout = 0;
  Logger::Instance()->Debug("before attempted connection, descriptor was: {}", this->driverConnection);
  ComDisconnect(this->driverConnection);
  this->driverConnection = ComOpen(baud, com_port);
  Logger::Instance()->Debug("after attempted connection, descriptor is: {}", this->driverConnection);
  if(this->driverConnection >= 0)
    SerialClear(this->driverConnection);
  return this->driverConnection >= 0;
}

bool Trinamic::TryConnectMotor()
{
  Logger::Instance()->Info("Attempting to connect to motor now at com port: {}, baud rate: {}", com_port, baud);
  int connection_attempts = 3;
  while(!ConnectMotor())
  {
    connection_attempts--;
    if(connection_attempts == 0)
    {
      Logger::Instance()->Error("Could not connect to Trinamic motion control board");
      exit(13);
    }
    TimerDelay(100); //ms delay
  }
  Logger::Instance()->Info("Successfully opened COM port to Trinamic motion control board");
  return true;
}

void Trinamic::PackLeValue(Trinamic_t* packet, uint32_t data)
{
	packet->Payload[4] = (data >> 24) & 0xFF;
	packet->Payload[5] = (data >> 16) & 0xFF;
	packet->Payload[6] = (data >> 8) & 0xFF;
	packet->Payload[7] = data & 0xFF;
}

void Trinamic::UnpackLeValue(Trinamic_t* packet)
{
	uint32_t value = packet->Rx.Value;
	packet->Payload[4] = (value >> 24) & 0xFF;
	packet->Payload[5] = (value >> 16) & 0xFF;
	packet->Payload[6] = (value >> 8) & 0xFF;
	packet->Payload[7] = value & 0xFF;
}

void Trinamic::CalcCrc(Trinamic_t* packet)
{
	packet->Payload[8] = 0;
	for(int i = 0; i < 8; i++)
    {
		packet->Payload[8] += packet->Payload[i];
	}
}

bool Trinamic::CheckRxPacket(Trinamic_t* packet)
{
  uint8_t crc = 0;
  for(int i = 0; i < 8; i++)
  {
      crc += packet->Payload[i];
  }
  if(crc != packet->Rx.Crc)
  {
    Logger::Instance()->Error("CheckRxPacket: Bad CRC: {} != {}", crc, packet->Rx.Crc);
    return false;
  }
  if(packet->Rx.Status != STATUS_SUCCESS)
  {
    Logger::Instance()->Error("CheckRxPacket: Bad status: {}", packet->Rx.Status);
    return false;
  }
  return true;
}

void Trinamic::UpdateMovingRegisters()
{
  if(SendMotorCommand(0x02, TMCL_GAP, MOT_ActualPosition, 0, 0)) {
    StepperReg.ActualPosition = TrinamicRx.Rx.Value;
  }
  if(SendMotorCommand(0x02, TMCL_GAP, MOT_ActualSpeed, 0, 0)) {
    StepperReg.ActualSpeed = TrinamicRx.Rx.Value;
  }
  if(SendMotorCommand(0x02, TMCL_GAP, MOT_PositionReached, 0, 0)) {
    StepperReg.PositionReached = TrinamicRx.Rx.Value;
  }
}

int Trinamic::GetAnalogSensorInput()
{
  if(SendMotorCommand(0x02, TMCL_GIO, 0, 1, 0))
  {
    return TrinamicRx.Rx.Value;
  }
  return -1;
}

int Trinamic::GetHomeSwitchState()
{
  if(SendMotorCommand(0x02, TMCL_GAP, MOT_HomeSwitch, 0, 0))
  {
    return TrinamicRx.Rx.Value;
  }
  return -1;
}

float Trinamic::GetLoadState()
{
  if(SendMotorCommand(0x02, TMCL_GAP, 206, 0, 0))
  {
    return TrinamicRx.Rx.Value;
  }
  return -1;
}

void Trinamic::StopMotor()
{
  SendMotorCommand(0x02,TMCL_MST, 0, 0, 0);
}

bool Trinamic::ReadReply()
{
	memset(&TrinamicRx, 0, sizeof(Trinamic_t));
	uint8_t c;
	int16_t total_bytes_read = 0;
	double timer = TimerMilliseconds();
	uint8_t bytesRead = 0;

    while(TimerMillisecondElapsed(timer) < 500)
    {
      if(BytesToRead(this->driverConnection) <= 0)
      {
        Logger::Instance()->Trace("No bytes to read yet...waiting a bit");
        usleep(1000);
        continue;
      }

      bytesRead = SerialRead(&c, 1, this->driverConnection);
      if(bytesRead != 1)
      {
        Logger::Instance()->Error("Trinamic serial read: tried to read 1 byte, but read: {} instead", bytesRead);
        continue;
      }

      TrinamicRx.Payload[total_bytes_read] = c;
      total_bytes_read += bytesRead;
      if(total_bytes_read >= TrinamicRx.Size())
      {
        UnpackLeValue(&TrinamicRx);
        return CheckRxPacket(&TrinamicRx);
      }
	}

    Logger::Instance()->Error("Bad response, Trinamic serial read. bank: {}, bytes_read: {}, of expected: {}", TrinamicTx.Tx.MotorBank, total_bytes_read, TrinamicRx.Size());

	for(int x = 0; x < TrinamicRx.Size(); x++)
    {
      Logger::Instance()->Debug("Printing out TrinamicRx.Payload, component {}: {}", x,  TrinamicRx.Payload[x]);
	}
	return false;
}

bool Trinamic::SendMotorCommand(uint8_t address, uint8_t command, uint8_t requestId, uint8_t motor, uint32_t value)
{
  this->timeout = 0;
  for(int t = 0; t < 5; t++)
  {
    memset(&TrinamicTx, 0, sizeof(Trinamic_t));
    TrinamicTx.Tx.Address = address;
    TrinamicTx.Tx.Command = command;
    TrinamicTx.Tx.RequestId = requestId;
    TrinamicTx.Tx.MotorBank = motor;
    PackLeValue(&TrinamicTx, value);
    CalcCrc(&TrinamicTx);

    Logger::Instance()->Trace("SerialWrite motor command in trinamic.cpp now");
    SerialWrite(TrinamicTx.Payload, TrinamicTx.Size(), this->driverConnection);

    Logger::Instance()->Trace("Attempting to read reply now");
    //int bytes = CanRead(TrinamicTx.Payload, TrinamicTx.Size());
    if (ReadReply())
    {
      Logger::Instance()->Trace("Successfully read reply after SendMotorCommand ", command, address);
      this->timeout = 0;
      return true;
    }

    this->timeout++;
    Logger::Instance()->Warn("Motion control command timeout. cmd: {}, address: {}", command, address);
    SerialClear(this->driverConnection);
    if (this->timeout > 2)
    {
      Logger::Instance()->Warn("Three timeouts during motor send command, will re-attempt to connect to motion controller now");
      TryConnectMotor();
      this->timeout = 0;
    }
  }
  Logger::Instance()->Warn("Too many timeouts reached, motion command failed.");
  return false;
}

void Trinamic::update_and_print_motor_status()
{
  UpdateMovingRegisters();
  Logger::Instance()->Info("Current motor position is: {}", StepperReg.ActualPosition);
  Logger::Instance()->Debug("Current motor speed is: {}", StepperReg.ActualSpeed);
  Logger::Instance()->Debug("Position reached flag is: {}", StepperReg.PositionReached);
}

int Trinamic::get_current_position()
{
  if(SendMotorCommand(0x02, TMCL_GAP, MOT_ActualPosition, 0, 0))
  {
   return TrinamicRx.Rx.Value;
  }
  else
  {
    return -1; //TODO: select default error value here that makes sense for application
  }
}

int Trinamic::check_position_reached()
{
  if(SendMotorCommand(0x02, TMCL_GAP, MOT_PositionReached, 0, 0))
  {
    return TrinamicRx.Rx.Value; //this will be a 0 or 1 as returned from Trinamic board
  }
  else
  {
    return -1; //TODO: select default error value here that makes sense for application
  }
}

void Trinamic::PrintTrinamicPacket(Trinamic_t* packet)
{
	printf("Addr: %d, Cmd: %d, Req: %d, Data %d, Status %d, Crc %d\n", packet->Rx.ReplyAddress,
			packet->Rx.Command, packet->Rx.ModuleAddress, packet->Rx.Value, TrinamicRx.Rx.Status, packet->Rx.Crc);
}
